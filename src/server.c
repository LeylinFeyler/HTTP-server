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
#include <unistd.h>

#define MAX_EVENTS 64

void handle_client(int client_fd, struct sockaddr_in *client) {
    char ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &client->sin_addr, ip, sizeof(ip));

    while (1) {
        char buffer[BUFFER_SIZE];
        char raw_buffer[BUFFER_SIZE];

        int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        /* client disconnected */
        if (n <= 0) {
            break;
        }

        buffer[n] = '\0';
        strcpy(raw_buffer, buffer);

        HttpRequest req;
        if (!parse_request(buffer, &req)) {
            send_error_page(client_fd, 400, "Bad Request", 0);
            break;
        }

        int keep_alive = should_keep_alive(&req);
        int send_body  = 1;
        if (strcmp(req.method, "HEAD") == 0) {
            send_body = 0;
        } else if (strcmp(req.method, "GET") != 0) {
            send_response(client_fd, 405, "Method Not Allowed", "405 Method Not Allowed",
                          keep_alive);
            if (!keep_alive) {
                break;
            }
            continue;
        }

        log_message(ip, req.method, req.path);
        log_raw_message(ip, req.method, req.path, raw_buffer);
        printf("%s %s %s\n", ip, req.method, req.path);

        if (!handle_route(client_fd, &req)) {
            send_file(client_fd, req.path, send_body, keep_alive);
        }

        /* close requested */
        if (!keep_alive) {
            break;
        }
    }
}

void run_server_epoll(void) {
    int server_fd = create_server_socket();

    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        exit(1);
    }

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events  = EPOLLIN;
    ev.data.fd = server_fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("epoll_ctl");
        exit(1);
    }

    struct epoll_event events[MAX_EVENTS];
    memset(&events, 0, sizeof(events));

    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            perror("epoll_wait");
            continue;
        }

        for (int i = 0; i < nfds; i++) {
            int current_fd = events[i].data.fd;

            if (current_fd == server_fd) {
                struct sockaddr_in client;
                int client_fd = accept_client(server_fd, &client);
                if (client_fd < 0) {
                    continue;
                }

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
            epoll_ctl(epfd, EPOLL_CTL_DEL, current_fd, NULL);

            close(current_fd);
        }
    }
}

int main(void) {
    run_server_epoll();

    return 0;
}