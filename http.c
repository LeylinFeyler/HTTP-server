#include "http.h"

#include <stdio.h>

int parse_request(char *raw, HttpRequest *req) {
    return sscanf(raw, "%15s %255s %31s", req->method, req->path, req->version);
}