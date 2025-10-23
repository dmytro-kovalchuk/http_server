#include "../include/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../include/utils.h"
#include "../include/file_storage.h"

volatile sig_atomic_t is_server_running = 1;
int g_server_fd = -1;

void start_server() {
    signal(SIGINT, handle_sigint);
    g_server_fd = create_file_descriptor();
    struct sockaddr_in server_addr = create_server_addr();
    bind_addr_to_socket(g_server_fd, server_addr);
    puts("Server is started. Press Ctrl+C to stop it...");
    handle_requests(g_server_fd);
    close(g_server_fd);
    g_server_fd = -1;
    puts("Server is stopped!");
}

int create_file_descriptor() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("Couldn't create socket");
        exit(EXIT_FAILURE);
    }

    make_port_reusable(server_fd);
    
    return server_fd;
}

void make_port_reusable(int server_fd) {
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Couldn't make server reuse port");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
}

struct sockaddr_in create_server_addr() {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(8080);
    return server_addr;
}

void bind_addr_to_socket(int server_fd, struct sockaddr_in server_addr) {
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Couldn't bind server address to file descriptor");
        exit(EXIT_FAILURE);
    }
}

void handle_requests(int server_fd) {
    start_listening(server_fd);
    while (is_server_running) {
        int client_socket = accept_connection(server_fd);
        if (client_socket == -1) break;
        handle_client(client_socket);
    }
}

void start_listening(int server_fd) {
    if (listen(server_fd, SOMAXCONN) == -1) {
        perror("Couldn't listen on socket");
        exit(EXIT_FAILURE);
    }
}

int accept_connection(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);

    if (client_socket == -1) {
        perror("Couldn't accept connection");
    }

    return client_socket;
}

void handle_client(int client_socket) {
    while (is_server_running) {
        char* raw_request = receive_request(client_socket);
        if (raw_request == NULL) break;

        struct Request request = parse_request(raw_request);
        
        if (strstr(request.headers, "Expect: 100-continue")) {
            const char* continue_response = "HTTP/1.1 100 Continue\r\n\r\n";
            send(client_socket, continue_response, strlen(continue_response), 0);
        }

        if (strcmp(request.method, "POST") == 0) {
            size_t content_len = parse_content_length(request.headers);

            int received_bytes = receive_file(client_socket,
                                  request.path,
                                  content_len,
                                  request.body,
                                  request.body_size);

            if (received_bytes != 0) {
                const char* error = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
                send(client_socket, error, strlen(error), 0);
                free(raw_request);
                if (request.body) free(request.body);
                break;
            }

            char* raw_response = create_response(request);
            send(client_socket, raw_response, strlen(raw_response), 0);
            free(raw_response);
        } else if (strcmp(request.method, "GET") == 0) {
            char* raw_response = create_response(request);
            send(client_socket, raw_response, strlen(raw_response), 0);
            free(raw_response);

            send_file(client_socket, request.path);
        } else if (strcmp(request.method, "DELETE") == 0) {
            char* raw_response = create_response(request);
            send(client_socket, raw_response, strlen(raw_response), 0);
            free(raw_response);
        } else {
            const char* raw_response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
            send(client_socket, raw_response, strlen(raw_response), 0);
        }

        if (request.body) free(request.body);
        free(raw_request);

        close(client_socket);
        break;
    }
}

char* receive_request(int client_socket) {
    size_t buffer_size = 8192;
    char* buffer = malloc(buffer_size + 1);
    if (!buffer) return NULL;

    size_t total_received_bytes = 0;
    while (1) {
        ssize_t received_bytes = recv(client_socket, buffer + total_received_bytes, buffer_size - total_received_bytes, 0);
        if (received_bytes <= 0) {
            free(buffer);
            return NULL;
        }

        total_received_bytes += (size_t)received_bytes;
        buffer[total_received_bytes] = '\0';

        char* header_end = strstr(buffer, "\r\n\r\n");
        if (header_end) {
            return buffer;
        }

        if (total_received_bytes >= buffer_size) {
            free(buffer);
            return NULL;
        }
    }
}

// void send_response(int client_socket, struct Request request);