#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "file.h"
#include "http.h"
#include "logger.h"

#define PORT 8888
#define BUFFER_SIZE 8192

void reap_zombies(int sig) {
    (void)sig;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

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

void handle_client(int client_fd, struct sockaddr_in *client) {
    char buffer[BUFFER_SIZE];

    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (n <= 0) {
        return;
    }

    buffer[n] = '\0';

    HttpRequest req;

    if (parse_request(buffer, &req) != 3) {
        send_response(client_fd, 400, "Bad Request", "400 Bad Request");
        return;
    }

    char ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &client->sin_addr, ip, sizeof(ip));

    log_message(ip, req.method, req.path);

    printf("%s %s %s\n", ip, req.method, req.path);

    if (strcmp(req.path, "/hello") == 0) {
        send_response(client_fd, 200, "OK", "hello user");
    } else {
        send_file(client_fd, req.path);
    }
}

int main() {
    signal(SIGCHLD, reap_zombies);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server;

    server.sin_family      = AF_INET;
    server.sin_port        = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&server, sizeof(server));

    listen(server_fd, SOMAXCONN);

    printf("http server on :%d\n", PORT);

    while (1) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);

        int fd = accept(server_fd, (struct sockaddr *)&client, &len);

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