// logger.c
#include <stdio.h>
#include <time.h>
#include "logger.h"
#include <string.h>


void log_request(const char *logfile, const char *client_ip, const char *method, const char *path, int status_code) {
    FILE *file = fopen(logfile, "a");
    if (!file) return;

    time_t now = time(0);
    char *dt = ctime(&now);
    dt[strcspn(dt, "\n")] = 0;

    fprintf(file, "[%s] %s \"%s %s\" %d\n", dt, client_ip, method, path, status_code);
    fclose(file);
}
