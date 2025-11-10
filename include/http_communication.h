/**
    * @file: http_communication.h
    * @author: Dmytro Kovalchuk
    *
    * This file contains declarations of functions responsible for
    * handling HTTP communication within the server.
    *
    * It provides functionality for parsing HTTP requests, generating
    * responses, and managing various HTTP methods such as GET, POST,
    * and DELETE. Additionally, it handles conversion between structured
    * response data and raw HTTP message strings.
*/

#ifndef HTTP_COMMUNICATION_H
#define HTTP_COMMUNICATION_H

#include "http_messages.h"

/**
    * Creates response and sends it.
    *
    * @param[in] client_socket The client socket descriptor.
    * @param[in] request The pointer to parsed Request structure.
    *
    * @return Returns 0 on success or error code on failure.
*/
enum ReturnCode handle_request(int client_socket, struct Request* request);

/**
    * Parses a raw HTTP request string into a structured Request object.
    *
    * @param[in] raw_request The raw HTTP request string.
    *
    * @return Returns a struct Request containing the parsed request data.
*/
struct Request parse_request(const char* raw_request);

/**
    * Determines whether the HTTP connection should remain open.
    *
    * @param[in] headers The HTTP headers list.
    *
    * @return Returns 1 if the connection should be kept alive,
    * or 0 if it should be closed.
*/
int is_keep_alive(const struct HeaderList headers);

/**
    * Deallocates memory of request and headers list.
    *
    * @param[in] request The pointer to Request structure.
*/
void free_request(struct Request* request);

#endif // HTTP_COMMUNICATION_H