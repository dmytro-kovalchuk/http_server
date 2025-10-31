/**
    * @file: main.c
    * @author: Dmytro Kovalchuk
    *
    * This file serves as the entry point for the server application.
    *
    * It initializes the server by calling the start_server() function,
    * which handles configuration loading, socket setup, and request
    * processing.
*/

#include "../include/server.h"

int main(void) {
    start_server();
    return 0;
}