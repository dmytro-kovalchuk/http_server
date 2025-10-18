#ifndef SERVER_H
#define SERVER_H

#define MIN_REQUEST_LEN 5

void start_server();

int create_file_descriptor();

struct sockaddr_in create_server_addr();

void bind_addr_to_socket(int server_fd, struct sockaddr_in server_addr);

void handle_requests(int server_fd);

void start_listening(int server_fd);

int accept_connection(int server_fd);

void handle_client();

char* receive_request();

void send_response();


#endif