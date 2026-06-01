#include "http.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

void trim_newline(char *s) {
    s[strcspn(s, "\r\n")] = '\0';
}

int should_keep_alive(HttpRequest *req) {
    /* HTTP/1.1 => keep-alive by default */
    if (strcmp(req->version, "HTTP/1.1") == 0) {
        if (strcasecmp(req->connection, "close") == 0) {
            return 0;
        }
        return 1;
    }

    /* HTTP/1.0 => only if requested */
    if (strcmp(req->version, "HTTP/1.0") == 0) {
        if (strcasecmp(req->connection, "keep-alive") == 0) {
            return 1;
        }
    }

    return 0;
}

int parse_request(char *raw, HttpRequest *req) {
    memset(req, 0, sizeof(HttpRequest));

    /* request body */
    char *body_start = strstr(raw, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        if (*body_start != '\0') {
            req->body = body_start;
        }
    }

    char *saveptr;
    char *line = strtok_r(raw, "\r\n", &saveptr);
    if (!line) {
        return 0;
    }

    if (sscanf(line, "%15s %255s %31s", req->method, req->path, req->version) != 3) {
        return 0;
    }

    /* validate method */
    if (!is_valid_method(req->method)) {
        return 0;
    }

    /* validate version */
    if (!is_valid_http_version(req->version)) {
        return 0;
    }

    /* path must start with / */
    if (req->path[0] != '/') {
        return 0;
    }

    /* split path + query */
    char *query_start = strchr(req->path, '?');
    if (query_start) {
        *query_start = '\0';
        query_start++;
        snprintf(req->query, sizeof(req->query), "%s", query_start);
    }

    while ((line = strtok_r(NULL, "\r\n", &saveptr))) {
        if (strncmp(line, "Host:", 5) == 0) {
            snprintf(req->host, sizeof(req->host), "%s", line + 6);
        } else if (strncmp(line, "User-Agent:", 11) == 0) {
            snprintf(req->user_agent, sizeof(req->user_agent), "%s", line + 12);
        } else if (strncmp(line, "Connection:", 11) == 0) {
            snprintf(req->connection, sizeof(req->connection), "%s", line + 12);
        } else if (strncmp(line, "Accept:", 7) == 0) {
            snprintf(req->accept, sizeof(req->accept), "%s", line + 8);
        } else if (strncmp(line, "Content-Length:", 15) == 0) {
            req->content_length = atoi(line + 16);
        } else if (strncmp(line, "Content-Type:", 13) == 0) {
            snprintf(req->content_type, sizeof(req->content_type), "%s", line + 14);
        } else if (strncmp(line, "Accept-Encoding:", 18) == 0) {
            snprintf(req->accept_encoding, sizeof(req->accept_encoding), "%s", line + 19);
        } else if (strncmp(line, "Referer:", 9) == 0) {
            snprintf(req->referer, sizeof(req->referer), "%s", line + 10);
        } else if (strncmp(line, "Origin:", 7) == 0) {
            snprintf(req->origin, sizeof(req->origin), "%s", line + 8);
        } else if (strncmp(line, "Authorization:", 14) == 0) {
            snprintf(req->authorization, sizeof(req->authorization), "%s", line + 15);
        } else if (strncmp(line, "Cookie:", 7) == 0) {
            snprintf(req->cookie, sizeof(req->cookie), "%s", line + 8);
        }
    }

    trim_newline(req->host);
    trim_newline(req->user_agent);
    trim_newline(req->connection);
    trim_newline(req->accept);
    trim_newline(req->content_type);
    trim_newline(req->accept_encoding);
    trim_newline(req->referer);
    trim_newline(req->origin);
    trim_newline(req->authorization);
    trim_newline(req->cookie);

    /* HTTP/1.1 requires Host */
    if (strcmp(req->version, "HTTP/1.1") == 0 && req->host[0] == '\0') {
        return 0;
    }

    /* POST requires Content-Length */
    if (strcmp(req->method, "POST") == 0 && req->content_length <= 0) {
        return 0;
    }

    return 1;
}

char *get_query_param(HttpRequest *req, const char *key) {
    static char value[256];
    value[0] = '\0';

    if (req->query[0] == '\0') {
        return NULL;
    }

    char query_copy[512];
    snprintf(query_copy, sizeof(query_copy), "%s", req->query);

    char *saveptr = NULL;
    char *pair    = strtok_r(query_copy, "&", &saveptr);
    while (pair) {
        char *eq = strchr(pair, '=');
        if (eq) {
            *eq               = '\0';
            char *param_key   = pair;
            char *param_value = eq + 1;

            if (strcmp(param_key, key) == 0) {
                url_decode(value, param_value);
                return value;
            }
        }
        pair = strtok_r(NULL, "&", &saveptr);
    }

    return NULL;
}

void url_decode(char *dst, const char *src) {
    while (*src) {
        /* %20 -> space */
        if (*src == '%' && isxdigit((unsigned char)src[1]) && isdigit((unsigned char)src[2])) {
            char hex[3];

            hex[0] = src[1];
            hex[1] = src[2];
            hex[2] = '\0';

            *dst = (char)strtol(hex, NULL, 16);
            src += 3;
        }

        /* '+' -> space */
        else if (*src == '+') {
            *dst = ' ';
            src++;
        } else {
            *dst = *src;
            src++;
        }
        dst++;
    }
    *dst = '\0';
}

int is_valid_method(const char *method) {
    return strcmp(method, "GET") == 0 || strcmp(method, "HEAD") == 0 || strcmp(method, "POST") == 0;
}

int is_valid_http_version(const char *version) {
    return strcmp(version, "HTTP/1.0") == 0 || strcmp(version, "HTTP/1.1") == 0;
}

void send_json_response(int fd, int status, const char *status_text, const char *json,
                        int keep_alive) {
    char response[4096];

    snprintf(response, sizeof(response),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %ld\r\n"
             "Connection: %s\r\n"
             "\r\n"
             "%s",
             status, status_text, strlen(json), keep_alive ? "keep-alive" : "close", json);

    send(fd, response, strlen(response), 0);
}