#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include <unistd.h>

int send_file(int client_socket, const char* path);

int receive_file(int client_socket, const char* filename, size_t content_size,
                 const void* received_body, size_t received_body_size);

int delete_file(const char* filename);

int is_file_exists(const char* filename);

size_t get_file_size(const char* filename);

void set_file_location(char* output, const char* filename);

#endif