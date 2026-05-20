#include "file.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#define CHUNK_SIZE 4096

void send_file(int client_fd, const char *filename) {
    FILE *file = fopen(filename, "rb");

    if (!file) {
        const char *response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type:text/plain\r\n"
            "\r\n"
            "404 File not found";

        send(client_fd, response, strlen(response), 0);
        return;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char header[1024];

    snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type:text/html\r\n"
            "Content-Length:%ld\r\n"
            "\r\n",
            size);

    send(client_fd, header, strlen(header), 0);

    char buffer[CHUNK_SIZE];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0)
        send(client_fd, buffer, bytes, 0);

    fclose(file);
}