#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include "die.h"

void die(const char *fmt, ...)
{
    va_list argp;
    FILE *log_file;
    log_file = fopen(ERROR_LOG_FILE_PATH, "a");

    if (!log_file)
        exit(EXIT_FAILURE);

    va_start(argp, fmt);
    vfprintf(log_file, fmt, argp);
    va_end(argp);
    fclose(log_file);
    exit(EXIT_FAILURE);
}
