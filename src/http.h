#ifndef HTTP_H
#define HTTP_H

#define MAX_REQUEST_SIZE 8192

typedef struct {
    char method[16];
    char path[256];
    char version[32];

    char host[256];
    char user_agent[512];
    char connection[64];
    char accept[256];

    int content_length;
    char *body;
} HttpRequest;

int parse_request(char *raw, HttpRequest *req);
int should_keep_alive(HttpRequest *req);

#endif