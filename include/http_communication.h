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
    * Parses a raw HTTP request string into a structured Request object.
    *
    * @param[in] raw_request The raw HTTP request string.
    *
    * @return Returns a struct Request containing the parsed request data.
*/
struct Request parse_request(const char* raw_request);

/**
    * Creates an HTTP response string from a given Request.
    *
    * @param[in] request The parsed Request structure.
    *
    * @return Returns a dynamically allocated string representing
    * the HTTP response message.
    *
    * @note The caller is responsible for freeing the returned string.
*/
char* create_response(struct Request request);

/**
    * Handles an HTTP GET request.
    *
    * @param[in] request The parsed Request structure.
    *
    * @return Returns a Response structure containing the server's reply.
*/
struct Response handle_method_get(struct Request request);

/**
    * Handles an HTTP POST request.
    *
    * @return Returns a Response structure containing the server's reply.
*/
struct Response handle_method_post();

/**
    * Handles an HTTP DELETE request.
    *
    * @param[in] request The parsed Request structure.
    *
    * @return Returns a Response structure containing the server's reply.
*/
struct Response handle_method_delete(struct Request request);

/**
    * Handles unsupported or unknown HTTP methods.
    *
    * @return Returns a Response structure with an appropriate error status.
*/
struct Response handle_method_other();

/**
    * Converts a Response structure into a complete HTTP response string.
    *
    * @param[in] response The Response structure to convert.
    *
    * @return Returns a dynamically allocated string containing
    * the formatted HTTP response.
    *
    * @note The caller is responsible for freeing the returned string.
*/
char* convert_struct_to_string(struct Response response);

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
    * @param[in] request The Request structure.
*/
void free_request(struct Request* request);

#endif // HTTP_COMMUNICATION_H