#ifndef HTTP_H
#define HTTP_H

#define BUFFER_SIZE 8192

typedef struct {
    char method[16];
    char path[256];
    char version[32];
} HttpRequest;

int parse_request(char *raw, HttpRequest *req);

#endif