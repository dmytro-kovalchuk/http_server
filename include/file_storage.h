#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

int send_file(int client_socket, const char* path);

int receive_file(int client_socket, const char* filename, unsigned int file_size);

#endif