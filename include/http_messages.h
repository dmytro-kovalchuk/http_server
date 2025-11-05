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

#include "common.h"

typedef unsigned long size_t;

enum Method {
    UNKNOWN,
    GET,
    POST,
    DELETE
};

/**
    * @struct Request
    * @brief Represents an HTTP request received from a client.
*/
struct Request {
    enum Method method;                 /**< The HTTP method (e.g., GET, POST, DELETE). */
    char path[MAX_PATH_LEN];            /**< The requested path or resource URI. */
    char version[HTTP_VERSION_SIZE];    /**< The HTTP version (e.g., HTTP/1.1). */
    char headers[HTTP_HEADER_SIZE];     /**< The raw request headers. */
    size_t content_len;                 /**< Value of Content-Length header. */
    int has_expect_continue_header;     /**< Bool value to check existence of 'Expect: 100-Continue' field. */
    char* body;                         /**< Pointer to the request body (optional). */
    size_t body_size;                   /**< Size of the request body in bytes. */
};

/**
    * @struct Response
    * @brief Represents an HTTP response sent to a client.
    *
    * This structure contains the response status line, headers,
    * and optional body content returned to the client.
*/
struct Response {
    char status[HTTP_STATUS_SIZE];      /**< The HTTP status line (e.g., 200 OK). */
    char headers[HTTP_HEADER_SIZE];     /**< The HTTP response headers. */
    char* body;                         /**< Pointer to the response body (optional). */
    size_t body_size;                   /**< Size of the response body in bytes. */
};

#endif // HTTP_MESSAGES_h