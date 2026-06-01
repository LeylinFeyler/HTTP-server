#include "routes.h"
#include "http.h"
#include "response.h"
#include <stdio.h>
#include <string.h>

void send_response(int fd, int status, const char *status_text, const char *text, int keep_alive);

// route handlers
void handle_hello(int fd, HttpRequest *req) {
    send_response(fd, 200, "OK", "hello user", should_keep_alive(req));
}

void handle_health(int fd, HttpRequest *req) {
    send_response(fd, 200, "OK", "server healthy", should_keep_alive(req));
}

void handle_echo(int fd, HttpRequest *req) {
    if (!req->body) {
        send_response(fd, 400, "Bad Request", "missing request body", should_keep_alive((req)));
        return;
    }

    send_response(fd, 200, "OK", req->body, should_keep_alive(req));
}

void handle_greet(int fd, HttpRequest *req) {
    char *name = get_query_param(req, "name");
    if (!name) {
        name = "user";
    }

    char response[256];
    snprintf(response, sizeof(response), "hello %s", name);
    send_response(fd, 200, "OK", response, should_keep_alive(req));
}

// route table
Route routes[] = {{"/hello", handle_hello},
                  {"/health", handle_health},
                  {"/echo", handle_echo},
                  {"/greet", handle_greet}};

int handle_route(int fd, HttpRequest *req) {
    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
        if (strcmp(req->path, routes[i].path) == 0) {
            routes[i].handler(fd, req);
            return 1;
        }
    }

    return 0;
}