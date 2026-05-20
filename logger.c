#include "logger.h"

#include <stdio.h>
#include <time.h>

void log_message(const char *ip,
                 const char *method,
                 const char *path) {
    FILE *f = fopen("server.log", "a");
    if (!f)
        return;

    time_t now = time(NULL);

    fprintf(f,
            "[%ld] %s %s %s\n",
            now,
            ip,
            method,
            path);

    fclose(f);
}