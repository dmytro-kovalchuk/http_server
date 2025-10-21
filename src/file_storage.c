#include "../include/file_storage.h"

#include <stdio.h>

int send_file(int client_socket, const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        perror("Couldn't open file");
        return -1;
    }

    char buffer[BUFSIZ];
    size_t bytes_read;

    while((bytes_read = fread(buffer, 1, BUFSIZ, file)) > 0) {
        ssize_t bytes_sent = send(client_socket, buffer, bytes_read, 0);
        if (bytes_sent == -1) {
            perror("Failed to send file");
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}

int receive_file(int client_socket, const char* filename, size_t file_size) {
    char path[256];
    snprintf(path, sizeof(path), "storage/%s", filename);

    FILE* file = fopen(path, "wb");
    if (file == NULL) {
        perror("Couldn't create file");
        return -1;
    }
    
    char buffer[BUFSIZ];
    size_t remaining_bytes = file_size;
    while (remaining_bytes > 0) {
        ssize_t received_bytes = recv(client_socket, buffer, BUFSIZ, 0);
        if (received_bytes <= 0) {
            perror("Failed to receive file");
            fclose(file);
            return -1;
        }

        fwrite(buffer, 1, received_bytes, file);
        remaining_bytes -= received_bytes;
    }

    fclose(file);
    return 0;
}