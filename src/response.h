#ifndef RESPONSE_H
#define RESPONSE_H

void send_response(int fd, int status, const char *status_text, const char *text);

#endif