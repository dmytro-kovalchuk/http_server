/**
    * @file: server.c
    * @author: Dmytro Kovalchuk
    *
    * This file contains the implementation of the server logic,
    * responsible for managing socket creation, configuration, and
    * lifecycle control of client connections.
    *
    * It implements multithreaded request handling using POSIX threads
    * and integrates with HTTP parsing, logging, and file storage modules
    * to process and respond to HTTP client requests.
    *
    * The server supports handling of HTTP GET, POST, and DELETE methods,
    * connection timeouts, Keep-Alive sessions, and safe shutdown
    * on termination signals.
*/

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
#include <arpa/inet.h>
#include "../include/http_header.h"
#include "../include/utils.h"
#include "../include/file_storage.h"
#include "../include/logger.h"
#include "../include/config.h"

volatile sig_atomic_t is_server_running = 1;
int g_server_fd = -1;

volatile unsigned int active_clients = 0;
pthread_mutex_t client_count_mutex = PTHREAD_MUTEX_INITIALIZER;

static void send_method_continue(int client_socket) {
    const char* continue_response = "HTTP/1.1 100 Continue\r\n\r\n";
    size_t response_len = strlen(continue_response);
    ssize_t bytes_sent = send(client_socket, continue_response, response_len, 0);

    if (bytes_sent < 0) {
        LOG_ERROR("Failed to send 100 Continue response");
        return;
    }

    if ((size_t)bytes_sent < response_len) {
        LOG_WARN("Partial send of 100 Continue");
    } else {
        LOG_INFO("100 Continue response sent successfully");
    }
}

static int send_method_post(int client_socket, struct Request request) {
    const char* expect_header = get_header_value(&request.headers, "Expect");
    if (expect_header && strcmp(expect_header, "100-continue") == 0) {
        send_method_continue(client_socket);
    }

    const char* content_len_str = get_header_value(&request.headers, "Content-Length");
    size_t content_len = content_len_str ? atoi(content_len_str) : 0;
    if (receive_file(client_socket, request.path, content_len, request.body, request.body_size) != RET_SUCCESS) {
        const char* error = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, error, strlen(error), 0);
        LOG_ERROR("Failed to receive file");
        return RET_ERROR;
    }

    char* raw_response = create_response(request);
    send(client_socket, raw_response, strlen(raw_response), 0);
    free(raw_response);

    LOG_INFO("POST method response sent");
    return RET_SUCCESS;
}

static void send_method_get(int client_socket, struct Request request) {
    char* raw_response = create_response(request);
    send(client_socket, raw_response, strlen(raw_response), 0);
    free(raw_response);
    LOG_INFO("GET method response sent");
    if (send_file(client_socket, request.path) != RET_SUCCESS) {
        LOG_ERROR("Failed to send file");
    }
}

static void send_method_delete(int client_socket, struct Request request) {
    char* raw_response = create_response(request);
    send(client_socket, raw_response, strlen(raw_response), 0);
    free(raw_response);
    LOG_INFO("DELETE method response sent");
}

static void send_method_other(int client_socket) {
    const char* raw_response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
    send(client_socket, raw_response, strlen(raw_response), 0);
    LOG_WARN("Other method response sent");
}

static void send_response(int client_socket, struct Request request) {
    switch (request.method) {
        case GET: send_method_get(client_socket, request); break;
        case POST: send_method_post(client_socket, request); break;
        case DELETE: send_method_delete(client_socket, request); break;
        case UNKNOWN: 
        default: send_method_other(client_socket);
    }
}

static void make_port_reusable(int server_fd) {
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == RET_ERROR) {
        LOG_FATAL("Couldn't make server reuse port");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Made possible for server to reuse port");
}

static int create_file_descriptor() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == RET_ERROR) {
        LOG_FATAL("Couldn't create socket");
        exit(EXIT_FAILURE);
    }

    make_port_reusable(server_fd);
    
    LOG_INFO("Server file descriptor(socket) created successfully");
    return server_fd;
}

static struct sockaddr_in create_server_addr() {
    const struct Config* config = get_config();

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config->port);
    server_addr.sin_addr.s_addr = htonl(config->ip);
    LOG_INFO("Created server address struct");
    return server_addr;
}

static void bind_addr_to_socket(int server_fd, struct sockaddr_in server_addr) {
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == RET_ERROR) {
        LOG_FATAL("Couldn't bind server address to file descriptor");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Bound server address to file descriptor");
}

static int set_client_timeout(int client_socket) {
    struct timeval timeout = {5, 0};

    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == RET_ERROR) {
        LOG_ERROR("Couldn't set timeout time for client");
        close(client_socket);
        return RET_ERROR;
    }

    LOG_INFO("Successfully set timeout time for client");
    return 0;
}

static void start_listening(int server_fd) {
    const struct Config* config = get_config();
    if (listen(server_fd, config->max_clients) == RET_ERROR) {
        LOG_FATAL("Couldn't listen on socket");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Start listening on socket");
}

static int accept_connection(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);

    if (client_socket == RET_ERROR) {
        LOG_ERROR("Couldn't accept connection");
    }

    LOG_INFO("Connection successfully accepted");
    return client_socket;
}

static char* receive_request(int client_socket) {
    size_t buffer_size = BUFSIZ;
    char* buffer = malloc(buffer_size + 1);
    if (buffer == NULL) {
        LOG_ERROR("Memory not allocated for raw request buffer");
        return NULL;
    }

    size_t total_received_bytes = 0;
    ssize_t received_bytes;
    while (1) {
        received_bytes = recv(client_socket, buffer + total_received_bytes, buffer_size - total_received_bytes, 0);
        if (received_bytes <= 0) {
            LOG_ERROR("Client disconnected or recv() error while reading headers");
            free(buffer);
            return NULL;
        }

        total_received_bytes += (size_t)received_bytes;
        buffer[total_received_bytes] = '\0';

        char* header_end = strstr(buffer, "\r\n\r\n");
        if (header_end != NULL) break;

        if (total_received_bytes >= buffer_size) {
            LOG_ERROR("Received bytes exceed buffer size while reading headers");
            free(buffer);
            return NULL;
        }
    }

    LOG_INFO("Received HTTP headers");
    return buffer;
}

static void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);

    while (1) {
        char* raw_request = receive_request(client_socket);
        if (raw_request == NULL) {
            LOG_WARN("Client closed connection or invalid request");
            break;
        }

        struct Request request = parse_request(raw_request);
        send_response(client_socket, request);
        free(raw_request);

        if (!is_keep_alive(request.headers)) {
            LOG_INFO("Connection: close - closing client socket");
            break;
        }

        LOG_INFO("Keep-Alive: waiting for next request on same connection");
        free_request(&request);
    }

    close(client_socket);
    LOG_INFO("Client socket closed");

    pthread_mutex_lock(&client_count_mutex);
    active_clients--;
    pthread_mutex_unlock(&client_count_mutex);

    return NULL;
}

static void handle_requests(int server_fd) {
    start_listening(server_fd);

    while (is_server_running) {
        int client_socket = accept_connection(server_fd);
        if (client_socket == RET_ERROR) break;
        if (set_client_timeout(client_socket) == RET_ERROR) break;

        const struct Config* config = get_config();
        pthread_mutex_lock(&client_count_mutex);
        if (active_clients > config->max_clients) {
            pthread_mutex_unlock(&client_count_mutex);
            LOG_WARN("Reached max clients count, connection rejected");
            close(client_socket);
            continue;
        }
        active_clients++;
        pthread_mutex_unlock(&client_count_mutex);

        int* client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_socket_ptr) != RET_SUCCESS) {
            LOG_ERROR("Couldn't create thread for new connection");
            close(client_socket);
            free(client_socket_ptr);

            pthread_mutex_lock(&client_count_mutex);
            active_clients--;
            pthread_mutex_unlock(&client_count_mutex);
        } else {
            pthread_detach(thread_id);
        }
    }
}

void server_start() {
    if (load_config("config.json") != RET_SUCCESS) {
        puts("Failed to load config");
    }

    if (initialize_logger() != RET_SUCCESS) return;

    g_server_fd = create_file_descriptor();
    struct sockaddr_in server_addr = create_server_addr();
    bind_addr_to_socket(g_server_fd, server_addr);
    
    puts("Server is started. Press Ctrl+C to stop it...");
    LOG_INFO("Server is started");
    handle_requests(g_server_fd);
}

void server_stop() {
    close(g_server_fd);
    g_server_fd = -1;
    deinitialize_logger();
    LOG_INFO("Server is stopped!");
}