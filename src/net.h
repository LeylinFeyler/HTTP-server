#ifndef NET_H
#define NET_H

#include <netinet/in.h>

int create_server_socket(void);
int accept_client(int server_fd, struct sockaddr_in *client);
int set_nonblocking(int fd);

#endif