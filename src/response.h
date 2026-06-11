#ifndef RESPONSE_H
#define RESPONSE_H

void send_response(int fd, int status, const char *status_text, const char *text, int keep_alive);
void send_method_not_allowed(int fd, int keep_alive);

#endif