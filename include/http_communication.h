/**
    * @file: http_communication.h
    * @author: Dmytro Kovalchuk
    *
    * This file contains declarations of structures and functions
    * responsible for handling HTTP communication within the server.
    *
    * It provides functionality for parsing HTTP requests, generating
    * responses, and managing various HTTP methods such as GET, POST,
    * and DELETE. Additionally, it handles conversion between structured
    * response data and raw HTTP message strings.
*/

#ifndef HTTP_COMMUNICATION_H
#define HTTP_COMMUNICATION_H

typedef unsigned long size_t;

/**
    * @struct Request
    * @brief Represents an HTTP request received from a client.
*/
struct Request {
    char method[16];       /**< The HTTP method (e.g., GET, POST, DELETE). */
    char path[512];        /**< The requested path or resource URI. */
    char version[32];      /**< The HTTP version (e.g., HTTP/1.1). */
    char headers[8192];    /**< The raw request headers. */
    char* body;            /**< Pointer to the request body (optional). */
    size_t body_size;      /**< Size of the request body in bytes. */
};

/**
    * @struct Response
    * @brief Represents an HTTP response sent to a client.
    *
    * This structure contains the response status line, headers,
    * and optional body content returned to the client.
*/
struct Response {
    char status[64];       /**< The HTTP status line (e.g., 200 OK). */
    char headers[256];     /**< The HTTP response headers. */
    char* body;            /**< Pointer to the response body (optional). */
    size_t body_size;      /**< Size of the response body in bytes. */
};

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
    * Extracts the Content-Length value from HTTP headers.
    *
    * @param[in] headers The raw HTTP headers string.
    *
    * @return Returns the content length value in bytes, or 0 if not found.
*/
size_t parse_content_length(const char* headers);

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
    * @param[in] headers The raw HTTP headers string.
    *
    * @return Returns 1 if the connection should be kept alive,
    * or 0 if it should be closed.
*/
int is_keep_alive(const char* headers);

#endif // HTTP_COMMUNICATION_H