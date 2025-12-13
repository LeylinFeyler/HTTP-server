#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd;
    int client_fd;
    struct sockaddr_in address;     
    int addrlen = sizeof(address);  
    char buffer[BUFFER_SIZE];       

    // створення TCP-сокета
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { 
        perror("socket"); 
        exit(EXIT_FAILURE); 
    }  

    // прив'язка сокета до адреси
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // перехід у режим очікування
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // підключення до клієнта
        client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_fd < 0) { 
            perror("accept"); 
            continue; 
        }

        // зчитування http-запиту
        int n = read(client_fd, buffer, BUFFER_SIZE-1);
        if (n < 0) { 
            perror("read"); 
            close(client_fd); 
            continue; 
        }
        buffer[n] = '\0';
        printf("Request:\n%s\n", buffer);

        // відправка http-відповіді
        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 12\r\n"
            "\r\n"
            "Hello World!\n";
        write(client_fd, response, strlen(response));

        // закриття клієнтського сокета
        close(client_fd);
    }

    // закритття серверного сокета
    close(server_fd);
    return 0;
}
