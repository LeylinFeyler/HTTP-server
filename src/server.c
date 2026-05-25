#include "file.h"
#include "http.h"
#include "logger.h"
#include "net.h"
#include "response.h"
#include "routes.h"

#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 8192

void reap_zombies(int sig) {
    (void)sig;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

void handle_client(int client_fd, struct sockaddr_in *client) {
    char buffer[BUFFER_SIZE], raw_buffer[BUFFER_SIZE];
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        return;
    }

    buffer[n] = '\0';
    strcpy(raw_buffer, buffer);

    HttpRequest req;
    if (!parse_request(buffer, &req)) {
        send_error_page(client_fd, 400, "Bad Request");
        return;
    }

    int send_body = 1;

    if (strcmp(req.method, "HEAD") == 0) {
        send_body = 0;
    } else if (strcmp(req.method, "GET") != 0) {
        send_response(client_fd, 405, "Method Not Allowed", "405 Method Not Allowed");
        return;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client->sin_addr, ip, sizeof(ip));

    log_message(ip, req.method, req.path);
    log_raw_message(ip, req.method, req.path, raw_buffer);
    printf("%s %s %s\n", ip, req.method, req.path);

    if (!handle_route(client_fd, &req)) {
        send_file(client_fd, req.path, send_body);
    }
}

int main(void) {
    signal(SIGCHLD, reap_zombies);

    int server_fd = create_server_socket();

    while (1) {
        struct sockaddr_in client;

        int fd = accept_client(server_fd, &client);
        if (fd < 0) {
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            close(server_fd);

            handle_client(fd, &client);

            close(fd);
            exit(0);
        }

        close(fd);
    }
}