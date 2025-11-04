/**
    * @file: main.c
    * @author: Dmytro Kovalchuk
    *
    * This file serves as the entry point for the server application.
    *
    * It sets custom function to handle SIGINT, initializes the
    * server by calling the server_start() function which handles
    * configuration loading, socket setup, and request processing,
    * and then stops the server using server_stop().
*/

#include <signal.h>
#include "../include/server.h"
#include "../include/utils.h"

int main(void) {
    signal(SIGINT, handle_sigint);
    server_start();
    server_stop();
    return 0;
}