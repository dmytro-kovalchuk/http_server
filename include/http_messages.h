/**
    * @file: http_messages.h
    * @author: Dmytro Kovalchuk
    *
    * This file contains declarations of structures used for
    * HTTP communication.
    *
    * It contains structure used for containing parsed requests
    * and responses.
*/

#ifndef HTTP_MESSAGES_H
#define HTTP_MESSAGES_H

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

#endif // HTTP_MESSAGES_h