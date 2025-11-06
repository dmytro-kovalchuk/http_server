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
#include <pthread.h>
#include "../include/logger.h"
#include "../include/config.h"
#include "../include/common.h"

static pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

enum ReturnCode send_file(int client_socket, const char* filename) {
    if (filename == NULL) {
        LOG_ERROR("Filename is NULL");
        return RET_ARGUMENT_IS_NULL;
    }

    char path[MAX_PATH_LEN];
    if (set_file_location(path, filename) != RET_SUCCESS) {
        return RET_ERROR;
    }

    pthread_mutex_lock(&file_mutex); // Lock before reading file to prevent deletion during send
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        pthread_mutex_unlock(&file_mutex);
        LOG_ERROR("Couldn't open file");
        return RET_FILE_NOT_OPENED;
    }
    pthread_mutex_unlock(&file_mutex); // Unlock after opening file

    char buffer[BUFSIZ];
    size_t bytes_read;

    while((bytes_read = fread(buffer, 1, BUFSIZ, file)) > 0) {
        size_t total_sent = 0;
        while (total_sent < bytes_read) {
            ssize_t bytes_sent = send(client_socket, buffer + total_sent, bytes_read - total_sent, 0);
            if (bytes_sent <= 0) {
                LOG_ERROR("Failed to send file");
                fclose(file);
                return RET_ERROR;
            }
            total_sent += bytes_sent;
        }
    }

    LOG_INFO("File was successfully sent");
    fclose(file);
    return RET_SUCCESS;
}

enum ReturnCode receive_file(int client_socket, const char* filename, size_t file_size,
                             const void* received_body, size_t received_body_size) {
    if (filename == NULL) {
        LOG_ERROR("Filename is NULL");
        return RET_ARGUMENT_IS_NULL;
    }

    char path[MAX_PATH_LEN];
    if (set_file_location(path, filename) != RET_SUCCESS) {
        return RET_ERROR;
    }

    pthread_mutex_lock(&file_mutex);
    FILE* file = fopen(path, "wb");
    if (file == NULL) {
        pthread_mutex_unlock(&file_mutex);
        LOG_ERROR("Couldn't create file");
        return RET_FILE_NOT_OPENED;
    }

    size_t remaining_bytes = file_size;

    if (received_body && received_body_size > 0) {
        if (fwrite(received_body, 1, received_body_size, file) != received_body_size) {
            LOG_ERROR("Couldn't write received body into file");
            fclose(file);
            pthread_mutex_unlock(&file_mutex);
            return RET_ERROR;
        }
        remaining_bytes -= received_body_size;
    }

    char buffer[BUFSIZ];
    while (remaining_bytes > 0) {
        size_t data_chunk = remaining_bytes < sizeof(buffer) ? remaining_bytes : sizeof(buffer);

        ssize_t received_bytes = recv(client_socket, buffer, data_chunk, 0);
        if (received_bytes <= 0) {
            LOG_ERROR("Failed during receiving data chunk");
            fclose(file);
            pthread_mutex_unlock(&file_mutex);
            return RET_ERROR;
        }

        if (fwrite(buffer, 1, (size_t)received_bytes, file) != (size_t)received_bytes) {
            LOG_ERROR("Couldn't write received data chunk into file");
            fclose(file);
            pthread_mutex_unlock(&file_mutex);
            return RET_ERROR;
        }
        remaining_bytes -= (size_t)received_bytes;
    }

    LOG_INFO("File was successfully received");
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
    return RET_SUCCESS;
}

int delete_file(const char* filename) {
    if (filename == NULL) {
        LOG_ERROR("Filename is NULL");
        return RET_ARGUMENT_IS_NULL;
    }

    char path[MAX_PATH_LEN];
    if (set_file_location(path, filename) != RET_SUCCESS) {
        return RET_ERROR;
    }

    pthread_mutex_lock(&file_mutex);
    int result = remove(path);
    pthread_mutex_unlock(&file_mutex);

    return result;
}

enum ReturnCode check_file_exists(const char* filename) {
    if (filename == NULL) {
        LOG_ERROR("Filename is NULL");
        return RET_ARGUMENT_IS_NULL;
    }

    char path[MAX_PATH_LEN];
    if (set_file_location(path, filename) != RET_SUCCESS) {
        return RET_ERROR;
    }

    FILE* file = fopen(path, "r");
    if (file != NULL) {
        fclose(file);
        return RET_SUCCESS;
    }

    return RET_ERROR;
}

size_t get_file_size(const char* filename) {
    if (filename == NULL) {
        LOG_ERROR("Filename is NULL");
        return 0;
    }

    char path[MAX_PATH_LEN];
    if (set_file_location(path, filename) != RET_SUCCESS) {
        return 0;
    }

    FILE* file = fopen(path, "rb");
    if (file == NULL) return 0;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    fclose(file);
    return size;
}

enum ReturnCode set_file_location(char* output, const char* filename) {
    if (filename == NULL) {
        LOG_ERROR("Filename is NULL");
        output = NULL;
        return RET_ARGUMENT_IS_NULL;
    }

    const struct Config* config = get_config();
    int written_bytes = snprintf(output, MAX_PATH_LEN, "%s%s", config->root_directory, filename);

    if (written_bytes < 0) {
        LOG_ERROR("Error creating file path in storage");
        output = NULL;
        return RET_ERROR;
    } else if (written_bytes >= MAX_PATH_LEN) {
        LOG_ERROR("File path is bigger than buffer size");
        output = NULL;
        return RET_ERROR;
    }

    return RET_SUCCESS;
}