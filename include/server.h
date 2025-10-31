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

/**
    * Creates a socket file descriptor for the server.
    *
    * @return Returns the created socket file descriptor,
    * or -1 on failure.
*/
int create_file_descriptor();

/**
    * Configures the server socket to allow port reuse.
    *
    * @param[in] server_fd The server socket file descriptor.
*/
void make_port_reusable(int server_fd);

/**
    * Creates and initializes a sockaddr_in structure
    * for the server’s address configuration.
    *
    * @return Returns a configured sockaddr_in structure.
*/
struct sockaddr_in create_server_addr();

/**
    * Binds the specified address to the server socket.
    *
    * @param[in] server_fd The server socket file descriptor.
    * @param[in] server_addr The server address to bind.
*/
void bind_addr_to_socket(int server_fd, struct sockaddr_in server_addr);

/**
    * Handles incoming client requests by accepting connections
    * and spawning threads to process them concurrently.
    *
    * @param[in] server_fd The server socket file descriptor.
*/
void handle_requests(int server_fd);

/**
    * Sets a timeout on client socket operations to prevent
    * indefinite blocking during communication.
    *
    * @param[in] client_socket The client socket file descriptor.
    *
    * @return Returns 0 on success or -1 on failure.
*/
int set_client_timeout(int client_socket);

/**
    * Places the server socket into listening mode to accept
    * incoming client connections.
    *
    * @param[in] server_fd The server socket file descriptor.
*/
void start_listening(int server_fd);

/**
    * Accepts a new client connection.
    *
    * @param[in] server_fd The server socket file descriptor.
    *
    * @return Returns the client socket file descriptor,
    * or -1 on failure.
*/
int accept_connection(int server_fd);

/**
    * Handles communication with a single connected client.
    *
    * This function runs in a separate thread for each client,
    * receiving requests, processing them, and sending responses.
    *
    * @param[in] arg Pointer to the client socket descriptor.
    *
    * @return Returns NULL when the thread finishes.
*/
void* handle_client(void* arg);

/**
    * Receives an HTTP request from a client socket.
    *
    * @param[in] client_socket The client socket file descriptor.
    *
    * @return Returns a dynamically allocated string containing
    * the raw HTTP request, or NULL on failure.
    *
    * @note The caller is responsible for freeing the returned string.
*/
char* receive_request(int client_socket);

/**
    * Sends an HTTP response to the client based on the parsed request.
    *
    * @param[in] client_socket The client socket descriptor.
    * @param[in] request The parsed HTTP request structure.
    *
    * @return Returns 0 on success or -1 on failure.
*/
int send_response(int client_socket, struct Request request);

/**
    * Sends an HTTP 100 Continue response to the client.
    *
    * @param[in] client_socket The client socket descriptor.
*/
void send_method_continue(int client_socket);

/**
    * Sends a response for an HTTP POST request.
    *
    * @param[in] client_socket The client socket descriptor.
    * @param[in] request The parsed HTTP request structure.
    *
    * @return Returns 0 on success or -1 on failure.
*/
int send_method_post(int client_socket, struct Request request);

/**
    * Sends a response for an HTTP GET request.
    *
    * @param[in] client_socket The client socket descriptor.
    * @param[in] request The parsed HTTP request structure.
*/
void send_method_get(int client_socket, struct Request request);

/**
    * Sends a response for an HTTP DELETE request.
    *
    * @param[in] client_socket The client socket descriptor.
    * @param[in] request The parsed HTTP request structure.s
*/
void send_method_delete(int client_socket, struct Request request);

/**
    * Sends a response for unsupported or unknown HTTP methods.
    *
    * @param[in] client_socket The client socket descriptor.
*/
void send_method_other(int client_socket);

#endif // SERVER_H