#include "../include/utils.h"

#include <signal.h>

extern volatile sig_atomic_t is_server_running;

void handle_sigint(int sig) {
    (void)sig;
    is_server_running = 0;
}