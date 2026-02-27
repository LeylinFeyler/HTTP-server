// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>

// #define PORT 8080
// #define BUFFER_SIZE 1024

// int main() {    
//     int server_fd;
//     int client_fd;
//     struct sockaddr_in6 address;     
//     int addrlen = sizeof(address);  
//     char buffer[BUFFER_SIZE];       

//     // створення TCP-сокета
//     server_fd = socket(AF_INET6, SOCK_STREAM, 0);
//     if (server_fd == -1) { 
//         perror("socket"); 
//         exit(EXIT_FAILURE); 
//     }  

//     // прив'язка сокета до адреси
//     memset(&address, 0, sizeof(address));
//     address.sin6_family = AF_INET6;
//     address.sin6_addr = in6addr_any; // 0.0.0.0
//     address.sin6_port = htons(PORT);
    

//     if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
//         perror("bind");
//         exit(EXIT_FAILURE);
//     }

//     // перехід у режим очікування
//     if (listen(server_fd, 10) < 0) {
//         perror("listen");
//         exit(EXIT_FAILURE);
//     }
    
//     printf("Server listening on port %d...\n", PORT);

//     while (1) {
//         // підключення до клієнта
//         client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
//         if (client_fd < 0) { 
//             perror("accept"); 
//             continue; 
//         }

//         // зчитування http-запиту
//         int n = read(client_fd, buffer, BUFFER_SIZE-1);
//         if (n < 0) { 
//             perror("read"); 
//             close(client_fd); 
//             continue; 
//         }
//         buffer[n] = '\0';
//         printf("Request:\n%s\n", buffer);

//         // відправка http-відповіді
//         const char* response =
//             "HTTP/1.1 200 OK\r\n"
//             "Content-Type: text/plain\r\n"
//             "Content-Length: 12\r\n"
//             "\r\n"
//             "Hello World!\n";
//         write(client_fd, response, strlen(response));

//         // закриття клієнтського сокета
//         close(client_fd);
//     }

//     // закритття серверного сокета
//     close(server_fd);
//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

#define PORT 8888
#define BACKLOG 10


void log_client_message(const char* ip, uint16_t port, const char* msg) {
    time_t now = time(NULL);
    char* timestr = ctime(&now);
    timestr[strlen(timestr)-1] = '\0';  // прибрати \n
    printf("[%s] %s:%u → %s\n", timestr, ip, port, msg);
}

void handle_client(int client_fd, const char* client_ip, uint16_t client_port) {
    char buffer[SOMAXCONN]; // 4096 з'єднань

    while (1) {
        ssize_t n = recv(client_fd, buffer, sizeof(buffer)-1, 0);
        if (n <= 0) {
            if (n < 0 && errno != EINTR) {
                perror("recv");
            }
            break;
        }

        buffer[n] = '\0';
        log_client_message(client_ip, client_port, buffer);

        // простий echo + додавання привітання
        char response[4608];
        snprintf(response, sizeof(response), "Сервер отримав: %s", buffer);
        send(client_fd, response, strlen(response), 0);
    }

    printf("Клієнт %s:%u завершив сеанс\n", client_ip, client_port);
    close(client_fd);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    //char buffer[1024];

    // 1. Створюємо сокет
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        exit(1);
    }

    // (часто корисно) дозволяємо швидко перезапускати сервер на тому ж порту
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Налаштовуємо адресу сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;       // 0.0.0.0 — всі інтерфейси
    server_addr.sin_port = htons(PORT);

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip,sizeof(client_ip));

    // 3. Прив’язуємо сокет до адреси
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    // 4. Переводимо в режим прослуховування
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen failed");
        exit(1);
    }

    printf("Сервер запущено на порту %d. Очікування з'єднань...\n", PORT);


    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }

        pid_t pid = fork();

        /*
        * fork() копіює таблицю файлових дескрипторів. Обидва процеси (батько і дитина) після fork()
        * мають server_fd і client_fd, які вказують на ті самі сокети в ядрі.
        */

        if (pid == 0) {
            /*
            * ДОЧІРНІЙ ПРОЦЕС
            *
            * Дитина обслуговує ТІЛЬКИ одного клієнта. Вона не повинна приймати нові підключення.
            *
            * server_fd (listening socket) дитині не потрібен, accept() має викликати лише батько.
            * Закриваємо, щоб:
            *  - не тримати зайвий ресурс
            *  - сокет реально закрився, коли сервер завершиться
            */
            close(server_fd);

            // Основна робота з клієнтом: recv()/send() через client_fd
            handle_client(client_fd, client_ip, htons(PORT));

            // Клієнт завершив роботу — закриваємо з'єднання. TCP-з'єднання закривається, коли всі fd закриті.
            close(client_fd);

            printf("Клієнт від'єднався\n");

            // Явно завершуємо дочірній процес, щоб він не повернувся в код батька.
            exit(0);

        } else if (pid > 0) {

            /*
            * БАТЬКІВСЬКИЙ ПРОЦЕС
            *
            * Батько:
            *  - слухає порт
            *  - приймає нові підключення
            *  - створює дочірні процеси
            *
            * client_fd більше не потрібен батькові — його обслуговує дочірній процес.
            * Якщо не закрити — TCP може не завершитись коректно.
            */
            close(client_fd);

            printf("Новий клієнт переданий дочірньому процесу\n");

        } else {

            /*
            * fork() завершився з помилкою.
            * Клієнта нікому обслуговувати — звільняємо ресурс.
            */
            perror("fork");
            close(client_fd);
            printf("Клієнт від'єднався\n");
        }
    }

    close(server_fd);
    return 0;
}