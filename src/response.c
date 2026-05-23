#include "response.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

void send_response(int fd, int status, const char *status_text, const char *text) {
    char response[4096];

    snprintf(response, sizeof(response),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type:text/plain\r\n"
             "Content-Length:%ld\r\n"
             "\r\n"
             "%s",
             status, status_text, strlen(text), text);

    send(fd, response, strlen(response), 0);
}