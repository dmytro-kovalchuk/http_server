/**
    * @file: utils.c
    * @author: Dmytro Kovalchuk
    *
    * This file contains implementation of utility functions
    * used across the server application.
    *
    * It provides functionality for handling system signals,
    * such as SIGINT, to ensure a graceful server shutdown by
    * properly terminating active connections and releasing
    * resources.
*/

#include "../include/utils.h"

#include <signal.h>
#include <sys/socket.h>
#include "../include/logger.h"

extern volatile sig_atomic_t is_server_running;
extern int g_server_fd;

void handle_sigint(int sig) {
    log_message(INFO, "Server shut down due to Ctrl+C");
    (void)sig;
    is_server_running = 0;
    if (g_server_fd != -1) shutdown(g_server_fd, SHUT_RDWR);
}