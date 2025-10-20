#include "../include/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

void start_server() {
    int server_fd = create_file_descriptor();
    struct sockaddr_in server_addr = create_server_addr();
    bind_addr_to_socket(server_fd, server_addr);
    handle_requests(server_fd);
    close(server_fd);
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
        close(server_fd);
        exit(EXIT_FAILURE);
    }
}

void handle_requests(int server_fd) {
    start_listening(server_fd);
    while (1) {
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

void handle_client() {
    while (1) {
        char* request = receive_request();
        if (request == NULL || strlen(request) < MIN_REQUEST_LEN) {
            free(request);
            break;
        }

        send_response();
        free(request);
    }
}

char* receive_request() {
    // TODO receive request
    return NULL;
}

void send_response() {
    // TODO send request
}