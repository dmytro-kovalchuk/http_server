#include "../include/utils.h"

#include <signal.h>
#include <sys/socket.h>

extern volatile sig_atomic_t is_server_running;
extern int g_server_fd;

void handle_sigint(int sig) {
    (void)sig;
    is_server_running = 0;
    if (g_server_fd != -1) shutdown(g_server_fd, SHUT_RDWR);
}