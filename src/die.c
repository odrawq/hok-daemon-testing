#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/*
 * Prints a formatted string to stderr and exits the program.
 */
void die(const char *fmt, ...)
{
    va_list argp;
	va_start(argp, fmt);
    fprintf(stderr, "\e[1;31m");
	vfprintf(stderr, fmt, argp);
    fprintf(stderr, "\e[0m");
	va_end(argp);
	exit(EXIT_FAILURE);
}
