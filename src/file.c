#include "file.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#define CHUNK_SIZE 4096

const char *get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');

    if (!ext) {
        return "application/octet-stream";
    }

    if (strcmp(ext, ".html") == 0) {
        return "text/html";
    }

    if (strcmp(ext, ".css") == 0) {
        return "text/css";
    }

    if (strcmp(ext, ".js") == 0) {
        return "application/javascript";
    }

    if (strcmp(ext, ".png") == 0) {
        return "image/png";
    }

    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
        return "image/jpeg";
    }

    if (strcmp(ext, ".svg") == 0) {
        return "image/svg+xml";
    }

    if (strcmp(ext, ".json") == 0) {
        return "application/json";
    }

    if (strcmp(ext, ".txt") == 0) {
        return "text/plain";
    }

    return "application/octet-stream";
}

void send_status_response(int fd, int status, const char *status_text, const char *text) {
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

void send_file(int client_fd, const char *path) {

    /* stop path traversal */
    if (strstr(path, "..")) {
        send_status_response(client_fd, 403, "Forbidden", "403 Forbidden");

        return;
    }

    char full_path[512];

    /* root route */
    if (strcmp(path, "/") == 0) {
        snprintf(full_path, sizeof(full_path), "static/index.html");
    } else {
        snprintf(full_path, sizeof(full_path), "static%s", path);
    }

    FILE *file = fopen(full_path, "rb");

    if (!file) {

        FILE *not_found = fopen("static/404.html", "rb");

        if (!not_found) {
            send_status_response(client_fd, 404, "Not Found", "404 File not found");

            return;
        }

        fclose(not_found);

        strcpy(full_path, "static/404.html");

        file = fopen(full_path, "rb");
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char header[1024];

    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type:%s\r\n"
             "Content-Length:%ld\r\n"
             "\r\n",
             get_mime_type(full_path), size);

    send(client_fd, header, strlen(header), 0);

    char buffer[CHUNK_SIZE];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {

        send(client_fd, buffer, bytes, 0);
    }

    fclose(file);
}