#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "die.h"
#include "bot.h"

static void handle_signal(int signal);
static void daemonize(void);

int main(void)
{
    signal(SIGSEGV, handle_signal);
    daemonize();
    start_bot();
    return 0;
}

static void handle_signal(int signal)
{
    (void) signal;
    die("Fatal: segmentation fault\n");
}

static void daemonize(void)
{
    pid_t pid;
    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    if (chdir("/"))
        die("Fatal: main.c: daemonize(): chdir() failed");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
