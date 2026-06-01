#include "file.h"
#include "http.h"
#include "logger.h"
#include "net.h"
#include "response.h"
#include "routes.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define MAX_EVENTS 64
#define MAX_CLIENTS 65536
#define KEEP_ALIVE_TIMEOUT 5

time_t client_last_activity[MAX_CLIENTS] = {0};

void handle_client(int client_fd, struct sockaddr_in *client) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client->sin_addr, ip, sizeof(ip));

    char buffer[MAX_REQUEST_SIZE];
    char raw_buffer[MAX_REQUEST_SIZE];

    int n = recv(client_fd, buffer, MAX_REQUEST_SIZE - 1, 0);
    /* graceful disconnect */
    if (n == 0) {
        printf("%s disconnected\n", ip);
        client_last_activity[client_fd] = 0;
        close(client_fd);
        return;
    }

    /* socket error */
    if (n < 0) {
        perror("recv");
        client_last_activity[client_fd] = 0;
        close(client_fd);
        return;
    }

    /* oversized request */
    if (n >= MAX_REQUEST_SIZE - 1) {
        send_response(client_fd, 413, "Payload Too Large", "413 Payload Too Large", 0);
        client_last_activity[client_fd] = 0;
        close(client_fd);
        return;
    }

    buffer[n] = '\0';
    strcpy(raw_buffer, buffer);

    HttpRequest req = {0};
    if (!parse_request(buffer, &req)) {
        send_error_page(client_fd, 400, "Bad Request", 0);
        client_last_activity[client_fd] = 0;
        close(client_fd);
        return;
    }

    client_last_activity[client_fd] = time(NULL);
    int keep_alive                  = should_keep_alive(&req);
    int send_body                   = 1;

    if (strcmp(req.method, "HEAD") == 0) {
        send_body = 0;
    } else if (strcmp(req.method, "GET") != 0) {
        send_response(client_fd, 405, "Method Not Allowed", "405 Method Not Allowed", keep_alive);
        if (!keep_alive) {
            client_last_activity[client_fd] = 0;
            close(client_fd);
        }
        return;
    }

    log_message(ip, req.method, req.path);
    log_raw_message(ip, req.method, req.path, raw_buffer);
    printf("%s %s %s\n", ip, req.method, req.path);

    if (!handle_route(client_fd, &req)) {
        send_file(client_fd, req.path, send_body, keep_alive);
    }

    /* client requested close */
    if (!keep_alive) {
        client_last_activity[client_fd] = 0;
        close(client_fd);
    }
}

void run_server_epoll(void) {
    int server_fd = create_server_socket();

    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        exit(1);
    }

    struct epoll_event ev = {0};
    ev.events             = EPOLLIN;
    ev.data.fd            = server_fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("epoll_ctl");
        exit(1);
    }

    struct epoll_event events[MAX_EVENTS] = {0};

    while (1) {
        /* cleanup idle clients */
        time_t now = time(NULL);

        for (int fd = 0; fd < MAX_CLIENTS; fd++) {
            if (client_last_activity[fd] == 0) {
                continue;
            }

            if ((now - client_last_activity[fd]) > KEEP_ALIVE_TIMEOUT) {
                printf("fd %d timeout\n", fd);
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
                client_last_activity[fd] = 0;
            }
        }

        int nfds = epoll_wait(epfd, events, MAX_EVENTS, 1000);
        if (nfds < 0) {
            perror("epoll_wait");
            continue;
        }

        for (int i = 0; i < nfds; i++) {
            int current_fd = events[i].data.fd;

            /* new client */
            if (current_fd == server_fd) {
                struct sockaddr_in client = {0};

                int client_fd = accept_client(server_fd, &client);
                if (client_fd < 0) {
                    continue;
                }

                client_last_activity[client_fd] = time(NULL);

                ev.events  = EPOLLIN;
                ev.data.fd = client_fd;

                if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
                    perror("epoll_ctl client");
                    close(client_fd);
                    continue;
                }

                continue;
            }

            struct sockaddr_in client;
            socklen_t len = sizeof(client);

            getpeername(current_fd, (struct sockaddr *)&client, &len);
            handle_client(current_fd, &client);
        }
    }
}

int main(void) {
    run_server_epoll();

    return 0;
}