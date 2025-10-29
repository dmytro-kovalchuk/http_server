#ifndef SERVER_H
#define SERVER_H

#include "../include/http_communication.h"

void start_server();

int create_file_descriptor();

void make_port_reusable(int server_fd);

struct sockaddr_in create_server_addr();

void bind_addr_to_socket(int server_fd, struct sockaddr_in server_addr);

void handle_requests(int server_fd);

int set_client_timeout(int client_socket);

void start_listening(int server_fd);

int accept_connection(int server_fd);

void* handle_client(void* arg);

char* receive_request(int client_socket);

int send_response(int client_socket, struct Request request);

void send_method_continue(int client_socket);

int send_method_post(int client_socket, struct Request request);

void send_method_get(int client_socket, struct Request request);

void send_method_delete(int client_socket, struct Request request);

void send_method_other(int client_socket);

#endif