#include "routes.h"
#include "file.h"
#include "response.h"
#include <string.h>

void send_response(int fd, int status, const char *status_text, const char *text);

// route handlers
void handle_hello(int fd, HttpRequest *req) {
    (void)req;
    send_response(fd, 200, "OK", "hello user");
}

void handle_health(int fd, HttpRequest *req) {
    (void)req;
    send_response(fd, 200, "OK", "server healthy");
}

// route table
Route routes[] = {{"/hello", handle_hello}, {"/health", handle_health}};

int handle_route(int fd, HttpRequest *req) {
    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
        if (strcmp(req->path, routes[i].path) == 0) {
            routes[i].handler(fd, req);
            return 1;
        }
    }

    return 0;
}