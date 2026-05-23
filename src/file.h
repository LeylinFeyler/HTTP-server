#ifndef FILE_H
#define FILE_H

const char *get_mime_type(const char *path);
void send_status_response(int fd, int status, const char *status_text, const char *text);
void send_file(int client_fd, const char *path);
int is_safe_path(const char *path);

#endif