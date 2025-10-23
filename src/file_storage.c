#include "../include/file_storage.h"

#include <stdio.h>
#include <sys/socket.h>
#include "../include/logger.h"

#define STORAGE_DIR "storage/"
#define MAX_FILE_PATH 512

int send_file(int client_socket, const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        log_message(ERROR, "Couldn't open file");
        return -1;
    }

    char buffer[BUFSIZ];
    size_t bytes_read;

    while((bytes_read = fread(buffer, 1, BUFSIZ, file)) > 0) {
        ssize_t bytes_sent = send(client_socket, buffer, bytes_read, 0);
        if (bytes_sent == -1) {
            log_message(ERROR, "Failed to send file");
            fclose(file);
            return -1;
        }
    }

    log_message(INFO, "File was successfully sent");
    fclose(file);
    return 0;
}

int receive_file(int client_socket, const char* filename, size_t file_size,
                 const void* received_body, size_t received_body_size) {
    char path[MAX_FILE_PATH];
    set_file_location(path, filename);

    FILE* file = fopen(path, "wb");
    if (!file) {
        log_message(ERROR, "Couldn't create file");
        return -1;
    }

    size_t remaining_bytes = file_size;

    if (received_body && received_body_size > 0) {
        fwrite(received_body, 1, received_body_size, file);
        remaining_bytes -= received_body_size;
    }

    char buffer[BUFSIZ];
    while (remaining_bytes > 0) {
        size_t data_chunk = remaining_bytes < sizeof(buffer) ? remaining_bytes : sizeof(buffer);

        ssize_t received_bytes = recv(client_socket, buffer, data_chunk, 0);
        if (received_bytes <= 0) {
            log_message(ERROR, "Failed during receiving data chunk");
            fclose(file);
            return -1;
        }

        fwrite(buffer, 1, (size_t)received_bytes, file);
        remaining_bytes -= (size_t)received_bytes;
    }

    log_message(INFO, "File was successfully received");
    fclose(file);
    return 0;
}

int delete_file(const char* filename) {
    char path[MAX_FILE_PATH];
    set_file_location(path, filename);
    return remove(path);
}

int is_file_exists(const char* filename) {
    char path[MAX_FILE_PATH];
    set_file_location(path, filename);

    FILE* file = fopen(path, "r");
    if (file != NULL) {
        fclose(file);
        return 1;
    }

    return 0;
}

size_t get_file_size(const char* filename) {
    char path[MAX_FILE_PATH];
    set_file_location(path, filename);

    FILE* file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

void set_file_location(char* output, const char* filenane) {
    snprintf(output, MAX_FILE_PATH, STORAGE_DIR "%s", filenane);
}