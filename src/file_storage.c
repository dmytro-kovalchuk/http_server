/**
    * @file: file_storage.c
    * @author: Dmytro Kovalchuk
    *
    * This file contains definitions of functions responsible for
    * handling file operations within the server application.
    *
    * It provides functionality for sending, receiving, deleting,
    * and verifying the existence of files. Additionally, it handles
    * file path resolution based on the server’s configured root
    * directory.
*/

#include "../include/file_storage.h"

#include <stdio.h>
#include <sys/socket.h>
#include "../include/logger.h"
#include "../include/config.h"
#include "../include/common.h"

int send_file(int client_socket, const char* filename) {
    if (filename == NULL) {
        log_message(ERROR, "Filename is NULL");
        return -1;
    }

    char path[MAX_PATH_LEN];
    set_file_location(path, filename);

    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        log_message(ERROR, "Couldn't open file");
        return -1;
    }

    char buffer[BUFSIZ];
    size_t bytes_read;

    while((bytes_read = fread(buffer, 1, BUFSIZ, file)) > 0) {
        size_t total_sent = 0;
        while (total_sent < bytes_read) {
            ssize_t bytes_sent = send(client_socket, buffer + total_sent, bytes_read - total_sent, 0);
            if (bytes_sent <= 0) {
                log_message(ERROR, "Failed to send file");
                fclose(file);
                return -1;
            }
            total_sent += bytes_sent;
        }
    }

    log_message(INFO, "File was successfully sent");
    fclose(file);
    return 0;
}

int receive_file(int client_socket, const char* filename, size_t file_size,
                 const void* received_body, size_t received_body_size) {
    if (filename == NULL) {
        log_message(ERROR, "Filename is NULL");
        return -1;
    }
    
                    char path[MAX_PATH_LEN];
    set_file_location(path, filename);

    FILE* file = fopen(path, "wb");
    if (file == NULL) {
        log_message(ERROR, "Couldn't create file");
        return -1;
    }

    size_t remaining_bytes = file_size;

    if (received_body && received_body_size > 0) {
        if (fwrite(received_body, 1, received_body_size, file) != received_body_size) {
            log_message(ERROR, "Couldn't write received body into file");
            return -1;
        }
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

        if (fwrite(buffer, 1, (size_t)received_bytes, file) != (size_t)received_bytes) {
            log_message(ERROR, "Couldn't write received data chunk into file");
            return -1;
        }
        remaining_bytes -= (size_t)received_bytes;
    }

    log_message(INFO, "File was successfully received");
    fclose(file);
    return 0;
}

int delete_file(const char* filename) {
    if (filename == NULL) {
        log_message(ERROR, "Filename is NULL");
        return -1;
    }

    char path[MAX_PATH_LEN];
    set_file_location(path, filename);
    return remove(path);
}

int is_file_exists(const char* filename) {
    if (filename == NULL) {
        log_message(ERROR, "Filename is NULL");
        return 0;
    }

    char path[MAX_PATH_LEN];
    set_file_location(path, filename);

    FILE* file = fopen(path, "r");
    if (file != NULL) {
        fclose(file);
        return 1;
    }

    return 0;
}

size_t get_file_size(const char* filename) {
    if (filename == NULL) {
        log_message(ERROR, "Filename is NULL");
        return 0;
    }

    char path[MAX_PATH_LEN];
    set_file_location(path, filename);

    FILE* file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

void set_file_location(char* output, const char* filename) {
    if (filename == NULL) {
        log_message(ERROR, "Filename is NULL");
        output = NULL;
        return;
    }

    const struct Config* config = get_config();
    snprintf(output, MAX_PATH_LEN,  "%s%s", config->root_directory, filename);
}