#ifndef SERVER_H
#define SERVER_H

#include "../include/http_communication.h"

#define MIN_REQUEST_LEN 5

void start_server();

int create_file_descriptor();

void make_port_reusable(int server_fd);

struct sockaddr_in create_server_addr();

void bind_addr_to_socket(int server_fd, struct sockaddr_in server_addr);

void handle_requests(int server_fd);

void start_listening(int server_fd);

int accept_connection(int server_fd);

void handle_client(int client_socket);

char* receive_request(int client_socket);

void send_response(int client_socket, struct Request request);

#endif