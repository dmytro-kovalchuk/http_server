#include "../include/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "../include/utils.h"
#include "../include/file_storage.h"
#include "../include/logger.h"

#define SERVER_PORT 8080

volatile sig_atomic_t is_server_running = 1;
int g_server_fd = -1;

void start_server() {
    signal(SIGINT, handle_sigint);
    g_server_fd = create_file_descriptor();
    struct sockaddr_in server_addr = create_server_addr();
    bind_addr_to_socket(g_server_fd, server_addr);
    
    puts("Server is started. Press Ctrl+C to stop it...");
    log_message(INFO, "Server is started");
    
    handle_requests(g_server_fd);
    close(g_server_fd);
    g_server_fd = -1;

    log_message(INFO, "Server is stopped!");
}

int create_file_descriptor() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        log_message(FATAL, "Couldn't create socket");
        exit(EXIT_FAILURE);
    }

    make_port_reusable(server_fd);
    
    log_message(INFO, "Server file descriptor(socket) created successfully");
    return server_fd;
}

void make_port_reusable(int server_fd) {
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        log_message(FATAL, "Couldn't make server reuse port");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    log_message(INFO, "Made possible for server to reuse port");
}

struct sockaddr_in create_server_addr() {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(SERVER_PORT);
    log_message(INFO, "Created server address struct");
    return server_addr;
}

void bind_addr_to_socket(int server_fd, struct sockaddr_in server_addr) {
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        log_message(FATAL, "Couldn't bind server address to file descriptor");
        exit(EXIT_FAILURE);
    }
    log_message(INFO, "Bound server address to file descriptor");
}

void handle_requests(int server_fd) {
    start_listening(server_fd);
    while (is_server_running) {
        int client_socket = accept_connection(server_fd);
        if (client_socket == -1) break;
        if (set_client_timeout(client_socket) == -1) break;

        int* client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_socket_ptr) != 0) {
            log_message(ERROR, "Couldn't create thread for new connection");
            close(client_socket);
            free(client_socket_ptr);
        } else {
            pthread_detach(thread_id);
        }
    }
}

int set_client_timeout(int client_socket) {
    struct timeval timeout = {5, 0};

    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        log_message(ERROR, "Couldn't set timeout time for client");
        close(client_socket);
        return -1;
    }

    log_message(INFO, "Successfully set timeout time for client");
    return 0;
}

void start_listening(int server_fd) {
    if (listen(server_fd, SOMAXCONN) == -1) {
        log_message(FATAL, "Couldn't listen on socket");
        exit(EXIT_FAILURE);
    }
    log_message(INFO, "Start listening on socket");
}

int accept_connection(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);

    if (client_socket == -1) {
        log_message(ERROR, "Couldn't accept connection");
    }

    log_message(INFO, "Connection successfully accepted");
    return client_socket;
}

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);

    while (1) {
        char* raw_request = receive_request(client_socket);
        if (raw_request == NULL) {
            log_message(WARN, "Client closed connection or invalid request");
            break;
        }

        struct Request request = parse_request(raw_request);
        send_response(client_socket, request);
        free(raw_request);

        if (!is_keep_alive(request.headers)) {
            log_message(INFO, "Connection: close - closing client socket");
            break;
        }

        log_message(INFO, "Keep-Alive: waiting for next request on same connection");
    }

    close(client_socket);
    log_message(INFO, "Client socket closed");
    return NULL;
}

char* receive_request(int client_socket) {
    size_t buffer_size = 8192;
    char* buffer = malloc(buffer_size + 1);
    if (buffer == NULL) {
        log_message(ERROR, "Memory not allocated for raw request buffer");
        return NULL;
    }

    size_t total_received_bytes = 0;
    ssize_t received_bytes;
    while (1) {
        received_bytes = recv(client_socket, buffer + total_received_bytes, buffer_size - total_received_bytes, 0);
        if (received_bytes <= 0) {
            log_message(ERROR, "Client disconnected or recv() error while reading headers");
            free(buffer);
            return NULL;
        }

        total_received_bytes += (size_t)received_bytes;
        buffer[total_received_bytes] = '\0';

        char* header_end = strstr(buffer, "\r\n\r\n");
        if (header_end != NULL) break;

        if (total_received_bytes >= buffer_size) {
            log_message(ERROR, "Received bytes exceed buffer size while reading headers");
            free(buffer);
            return NULL;
        }
    }

    size_t content_length = parse_content_length(buffer);
    size_t header_length = strstr(buffer, "\r\n\r\n") - buffer + 4;
    size_t expected_total = header_length + content_length;
    while (total_received_bytes < expected_total) {
        received_bytes = recv(client_socket, buffer + total_received_bytes, buffer_size - total_received_bytes, 0);
        if (received_bytes <= 0) {
            log_message(ERROR, "Client disconnected or recv() error while reading body");
            free(buffer);
            return NULL;
        }

        total_received_bytes += (size_t)received_bytes;
        buffer[total_received_bytes] = '\0';

        if (total_received_bytes >= buffer_size) {
            log_message(ERROR, "Received bytes exceed buffer size while reading body");
            free(buffer);
            return NULL;
        }
    }

    log_message(INFO, "Received complete HTTP request");
    return buffer;
}

int send_response(int client_socket, struct Request request) {
    if (strstr(request.headers, "Expect: 100-continue")) {
        send_method_continue(client_socket);
    }
    
    if (strcmp(request.method, "POST") == 0) {
        return send_method_post(client_socket, request);
    } else if (strcmp(request.method, "GET") == 0) {
        send_method_get(client_socket, request);
    } else if (strcmp(request.method, "DELETE") == 0) {
        send_method_delete(client_socket, request);
    } else {
        send_method_other(client_socket);
    }

    return 0;
}

void send_method_continue(int client_socket) {
    const char* continue_response = "HTTP/1.1 100 Continue\r\n\r\n";
    send(client_socket, continue_response, strlen(continue_response), 0);
    log_message(INFO, "CONTINUE method response sent");
}

int send_method_post(int client_socket, struct Request request) {
    size_t content_size = parse_content_length(request.headers);

    int received_bytes = receive_file(client_socket,
                            request.path,
                            content_size,
                            request.body,
                            request.body_size);

    if (received_bytes != 0) {
        const char* error = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, error, strlen(error), 0);
        if (request.body) free(request.body);
        log_message(ERROR, "Failed to receive file");
        return -1;
    }

    char* raw_response = create_response(request);
    send(client_socket, raw_response, strlen(raw_response), 0);
    free(raw_response);

    log_message(INFO, "POST method response sent");
    return 0;
}

void send_method_get(int client_socket, struct Request request) {
    char* raw_response = create_response(request);
    send(client_socket, raw_response, strlen(raw_response), 0);
    free(raw_response);
    log_message(INFO, "GET method response sent");
    send_file(client_socket, request.path);
}

void send_method_delete(int client_socket, struct Request request) {
    char* raw_response = create_response(request);
    send(client_socket, raw_response, strlen(raw_response), 0);
    free(raw_response);
    log_message(INFO, "DELETE method response sent");
}

void send_method_other(int client_socket) {
    const char* raw_response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
    send(client_socket, raw_response, strlen(raw_response), 0);
    log_message(WARN, "Other method response sent");
}