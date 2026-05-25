#include "response.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

void send_response(int fd, int status, const char *status_text, const char *text, int keep_alive) {
    char response[4096];

    snprintf(response, sizeof(response),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type:text/plain\r\n"
             "Content-Length:%ld\r\n"
             "Connection: %s\r\n"
             "\r\n"
             "%s",
             status, status_text, strlen(text), keep_alive ? "keep-alive" : "close", text);

    send(fd, response, strlen(response), 0);
}