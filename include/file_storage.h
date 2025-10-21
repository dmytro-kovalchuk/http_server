#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include <unistd.h>

int send_file(int client_socket, const char* path);

int receive_file(int client_socket, const char* filename, size_t file_size);

#endif