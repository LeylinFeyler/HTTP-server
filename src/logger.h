#ifndef LOGGER_H
#define LOGGER_H

void log_message(const char *ip, const char *method, const char *path);
void log_raw_message(const char *ip, const char *method, const char *path, const char *buffer);

#endif