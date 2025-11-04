/**
    * @file: file_storage.h
    * @author: Dmytro Kovalchuk
    *
    * This file contains declarations of functions responsible for
    * file management operations within the server application.
    *
    * It provides an interface for sending, receiving, deleting, and
    * checking files, as well as determining file size and resolving
    * file paths relative to the server’s configured root directory.
*/

#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include <unistd.h>

/**
    * Sends a file to the specified client socket.
    *
    * @param[in] client_socket The client socket descriptor.
    * @param[in] filename The name of the file to send.
    *
    * @return Returns 0 on success or error code on failure.
*/
enum ReturnCode send_file(int client_socket, const char* filename);

/**
    * Receives a file from the specified client socket.
    *
    * @param[in] client_socket The client socket descriptor.
    * @param[in] filename The name of the file to save as.
    * @param[in] content_size The total size of the file to receive.
    * @param[in] received_body Pointer to an optional buffer containing
    * the first part of the received data.
    * @param[in] received_body_size The size of the initial received data.
    *
    * @return Returns 0 on success or error code on failure.
*/
enum ReturnCode receive_file(int client_socket, const char* filename, size_t content_size,
                 const void* received_body, size_t received_body_size);

/**
    * Deletes a file from the server’s file system.
    *
    * @param[in] filename The name of the file to delete.
    *
    * @return Returns 0 on success or error code if the deletion fails.
*/
enum ReturnCode delete_file(const char* filename);

/**
    * Checks whether a file exists in the server’s storage.
    *
    * @param[in] filename The name of the file to check.
    *
    * @return Returns 0 if the file exists, or error code if it does not.
*/
enum ReturnCode check_file_exists(const char* filename);

/**
    * Retrieves the size of a file in bytes.
    *
    * @param[in] filename The name of the file.
    *
    * @return Returns the size of the file in bytes.
*/
size_t get_file_size(const char* filename);

/**
    * Constructs the full file path by combining the root directory
    * with the given filename.
    *
    * @param[out] output The buffer where the resolved file path is stored.
    * @param[in] filename The name of the file.
*/
enum ReturnCode set_file_location(char* output, const char* filename);

#endif // FILE_STORAGE_H