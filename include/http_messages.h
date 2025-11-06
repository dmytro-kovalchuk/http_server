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
    * @struct Header
    * @brief Represents HTTP header in packet.
*/
struct Header {
    char* key;
    char* value;
};

/**
    * @struct HeaderList
    * @brief Represents list of HTTP headers.
*/
struct HeaderList {
    struct Header* items;
    size_t size;
};

/**
    * @struct Request
    * @brief Represents HTTP request received from a client.
*/
struct Request {
    enum Method method;                 /**< The HTTP method (e.g., GET, POST, DELETE). */
    char path[MAX_PATH_LEN];            /**< The requested path or resource URI. */
    char version[HTTP_VERSION_SIZE];    /**< The HTTP version (e.g., HTTP/1.1). */
    struct HeaderList headers;          /**< Parsed headers as key-value pairs. */
    char* body;                         /**< Pointer to the request body (optional). */
    size_t body_size;                   /**< Size of the request body in bytes. */
};

/**
    * @struct Response
    * @brief Represents  HTTP response sent to a client.
    *
    * This structure contains the response status line, headers,
    * and optional body content returned to the client.
*/
struct Response {
    char status[HTTP_STATUS_SIZE];      /**< The HTTP status line (e.g., 200 OK). */
    struct HeaderList headers;          /**< Parsed headers as key-value pairs. */
    char* body;                         /**< Pointer to the response body (optional). */
    size_t body_size;                   /**< Size of the response body in bytes. */
};

#endif // HTTP_MESSAGES_h