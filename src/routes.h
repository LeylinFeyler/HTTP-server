#ifndef ROUTES_H
#define ROUTES_H

#include "http.h"

typedef void (*RouteHandler)(int fd, HttpRequest *req);

typedef struct {
    const char *path;
    RouteHandler handler;
} Route;

int handle_route(int fd, HttpRequest *req);

#endif