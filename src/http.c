#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        }
    }

    trim_newline(req->host);
    trim_newline(req->user_agent);
    trim_newline(req->connection);
    trim_newline(req->accept);

    return 1;
}