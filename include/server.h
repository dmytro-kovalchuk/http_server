/**
    * @file: server.h
    * @author: Dmytro Kovalchuk
    *
    * This header file declares the interfaces for the server module,
    * responsible for managing socket creation, configuration, and
    * lifecycle control of client connections.
    *
    * It defines functions for initializing the server, handling
    * incoming client requests, processing HTTP methods, and managing
    * connection behavior such as timeouts and Keep-Alive sessions.
    *
    * The server module integrates with other components including
    * HTTP communication, file storage, logging, and configuration
    * handling to support multithreaded request processing and safe
    * shutdown on termination signals.
*/
#ifndef SERVER_H
#define SERVER_H

#include "../include/http_communication.h"

/**
    * Starts the server and initializes all required components.
    *
    * This function sets up signal handling, loads configuration,
    * creates and binds the server socket, and begins handling client
    * requests until termination.
*/
void start_server();

#endif // SERVER_H