#include "http.h"

#include <stdio.h>
#include <string.h>

void trim_newline(char *s) {
    s[strcspn(s, "\r\n")] = '\0';
}

int parse_request(char *raw, HttpRequest *req) {
    memset(req, 0, sizeof(HttpRequest));

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
        }
    }

    trim_newline(req->host);
    trim_newline(req->user_agent);
    trim_newline(req->connection);
    trim_newline(req->accept);

    return 1;
}