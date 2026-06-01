#ifndef HTTP_H
#define HTTP_H

#define MAX_REQUEST_SIZE 8192
#define JSON_OK "{\"status\":\"ok\"}"
#define JSON_ERROR "{\"status\":\"error\"}"

typedef struct {
    char method[16];
    char path[256];
    char version[32];

    char query[512];

    char host[256];
    char user_agent[512];
    char connection[64];
    char accept[256];

    char content_type[128];
    char accept_encoding[128];
    char referer[512];
    char origin[256];
    char authorization[512];
    char cookie[1024];

    int content_length;
    char *body;
} HttpRequest;

int should_keep_alive(HttpRequest *req);
int parse_request(char *raw, HttpRequest *req);
char *get_query_param(HttpRequest *req, const char *key);
void url_decode(char *dst, const char *src);

int is_valid_method(const char *method);
int is_valid_http_version(const char *version);

void send_json_response(int fd, int status, const char *status_text, const char *json,
                        int keep_alive);

#endif