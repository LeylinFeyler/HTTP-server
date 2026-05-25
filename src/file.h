#ifndef FILE_H
#define FILE_H

const char *get_mime_type(const char *path);
void send_status_response(int fd, int status, const char *status_text, const char *text,
                          int keep_alive);
void send_file(int client_fd, const char *path, int send_body, int keep_alive);
int is_safe_path(const char *path);
void send_error_page(int client_fd, int status, const char *status_text, int keep_alive);

#endif