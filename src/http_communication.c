/**
    * @file: http_communication.c
    * @author: Dmytro Kovalchuk
    *
    * This file contains definitions of functions that handle
    * HTTP communication logic for the server application.
    *
    * It implements parsing of HTTP requests, generation of
    * appropriate HTTP responses, and handling of supported
    * methods such as GET, POST, and DELETE. Unsupported
    * methods result in an HTTP 405 response.
    *
    * Additionally, this file includes functionality for
    * converting response structures into raw HTTP strings,
    * extracting header information such as Content-Length,
    * and determining whether connections should be kept alive.
*/

#include "../include/http_communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/file_storage.h"
#include "../include/logger.h"
#include "../include/common.h"

#define METHOD_GET "GET"
#define METHOD_POST "POST"
#define METHOD_DELETE "DELETE"

static struct Request initialize_request() {
    struct Request request;
    memset(&request, 0, sizeof(request));
    request.method = UNKNOWN;
    request.body = NULL;
    request.body_size = 0;
    request.has_expect_continue_header = 0;
    return request;
}

struct Request parse_request(const char* raw_request) {
    struct Request request = initialize_request();

    if (raw_request == NULL) {
        LOG_WARN("Raw request is NULL");
        return request;
    }

    char method[METHOD_STR_LEN];
    int parsed = sscanf(raw_request, "%15s %511s %31s", method, request.path, request.version);
    if (parsed != 3) {
        LOG_ERROR("Couldn't parse raw request");
        return request;
    }

    if (strcmp(method, METHOD_GET) == RET_SUCCESS) {
        request.method = GET;
    } else if (strcmp(method, METHOD_POST) == RET_SUCCESS) {
        request.method = POST;
    } else if (strcmp(method, METHOD_DELETE) == RET_SUCCESS) {
        request.method = DELETE;
    } else {
        request.method = UNKNOWN;
    }

    const char* header_end = strstr(raw_request, "\r\n\r\n");
    if (header_end != NULL) {
        size_t header_size = header_end - raw_request;
        if (header_size >= sizeof(request.headers)) {
            header_size = sizeof(request.headers) - 1;
        }
        memcpy(request.headers, raw_request, header_size);
        request.headers[header_size] = '\0';

        const char* after_header = header_end + 4;
        size_t extra_bytes = strlen(after_header);
        if (extra_bytes > 0) {
            request.body = malloc(extra_bytes + 1);
            if (request.body != NULL) {
                memcpy(request.body, after_header, extra_bytes);
                request.body[extra_bytes] = '\0';
                request.body_size = extra_bytes;
            }
        } else {
            strncpy(request.headers, raw_request, sizeof(request.headers) - 1);
            request.headers[sizeof(request.headers) - 1] = '\0';
        }
    }

    request.content_len = parse_content_length(request.headers);

    if (strstr(request.headers, "Expect: 100-continue") != NULL) {
        request.has_expect_continue_header = 1;
    }

    LOG_INFO("Raw request parsed successfully");
    return request;
}

static struct Response initialize_response() {
    struct Response response;
    memset(&response, 0, sizeof(response));
    response.body = NULL;
    response.body_size = 0;
    return response;
}

char* create_response(struct Request request) {
    struct Response response;

    switch (request.method) {
        case GET: response = handle_method_get(request); break;
        case POST: response = handle_method_post(); break;
        case DELETE: response = handle_method_delete(request); break;
        case UNKNOWN: 
        default: response = handle_method_other();
    }

    if (is_keep_alive(request.headers)) {
        strcat(response.headers, "\r\nConnection: keep-alive\r\nKeep-Alive: timeout=5, max=100");
    } else {
        strcat(response.headers, "\r\nConnection: close");
    }

    return convert_struct_to_string(response);
}

size_t parse_content_length(const char* headers) {
    if (headers == NULL) {
        LOG_WARN("No headers detected");
        return 0;
    }

    const char* curr_line = headers;
    while (curr_line != NULL && *curr_line != '\0') {
        const char* line_end = strstr(curr_line, "\r\n");
        if (line_end == NULL) {
            line_end = curr_line + strlen(curr_line);
        }

        if (strncasecmp(curr_line, "Content-Length:", 15) == RET_SUCCESS) {
            const char* val = curr_line + 15;
            while (*val == ' ' || *val == '\t') val++;
            LOG_INFO("Found and parsed 'Content-Length' header");
            return (size_t)strtoul(val, NULL, 10);
        }

        if (*line_end == '\0') break;

        curr_line = line_end + 2;
    }

    LOG_INFO("No 'Content-Length' header detected");
    return 0;
}

struct Response handle_method_get(struct Request request) {
    struct Response response = initialize_response();

    if (check_file_exists(request.path) != RET_SUCCESS) {
        response.body = strdup("Not Found");
        if (response.body == NULL) {
            LOG_ERROR("Memory allocation failed while creating response body");
            response.body = "";
            response.body_size = 0;
            strcpy(response.status, STATUS_500_INTERNAL_SERVER_ERROR);
            snprintf(response.headers, sizeof(response.headers),
                    "Content-Type: text/plain\r\nContent-Length: %zu", response.body_size);
            return response;
        }
        response.body_size = strlen(response.body);
        LOG_WARN("GET method: file not found");
        strcpy(response.status, STATUS_404_NOT_FOUND);
        snprintf(response.headers, sizeof(response.headers), "Content-Type: text/plain\r\nContent-Length: %zu", response.body_size);
    } else {
        LOG_INFO("GET method: file exists");
        strcpy(response.status, STATUS_200_OK);
        snprintf(response.headers, sizeof(response.headers), "Content-Type: application/octet-stream\r\nContent-Length: %zu", get_file_size(request.path));
    }

    LOG_INFO("GET method response created");
    return response;
}

struct Response handle_method_post() {
    struct Response response = initialize_response();

    response.body = strdup("File created.\n");
    if (response.body == NULL) {
        LOG_ERROR("Memory allocation failed while creating response body");
        response.body = "";
        response.body_size = 0;
        strcpy(response.status, STATUS_500_INTERNAL_SERVER_ERROR);
        snprintf(response.headers, sizeof(response.headers),
                "Content-Type: text/plain\r\nContent-Length: %zu", response.body_size);
        return response;
    }
    response.body_size = strlen(response.body);

    LOG_INFO("POST method: file created");
    strcpy(response.status, STATUS_201_CREATED);
    snprintf(response.headers, sizeof(response.headers), "Content-Type: text/plain\r\nContent-Length: %zu", response.body_size);

    LOG_INFO("POST method response created");
    return response;
}

struct Response handle_method_delete(struct Request request) {
    struct Response response = initialize_response();

    if (delete_file(request.path) == RET_SUCCESS) {
        response.body = strdup("File deleted.\n");
        if (response.body == NULL) {
            LOG_ERROR("Memory allocation failed while creating response body");
            response.body = "";
            response.body_size = 0;
            strcpy(response.status, STATUS_500_INTERNAL_SERVER_ERROR);
            snprintf(response.headers, sizeof(response.headers),
                    "Content-Type: text/plain\r\nContent-Length: %zu", response.body_size);
            return response;
        }
        response.body_size = strlen(response.body);
        response.body_size = response.body != NULL ? strlen(response.body) : 0;
        LOG_INFO("DELETE method: file deleted");
        strcpy(response.status, STATUS_200_OK);
        snprintf(response.headers, sizeof(response.headers), "Content-Type: text/plain\r\nContent-Length: %zu", response.body_size);
    } else {
        LOG_WARN("DELETE method: file doesn't exists");
        response.body = strdup("Not Found");
        if (response.body == NULL) {
            LOG_ERROR("Memory allocation failed while creating response body");
            response.body = "";
            response.body_size = 0;
            strcpy(response.status, STATUS_500_INTERNAL_SERVER_ERROR);
            snprintf(response.headers, sizeof(response.headers),
                    "Content-Type: text/plain\r\nContent-Length: %zu", response.body_size);
            return response;
        }
        response.body_size = strlen(response.body);
        response.body_size = response.body != NULL ? strlen(response.body) : 0;
        strcpy(response.status, STATUS_404_NOT_FOUND);
        snprintf(response.headers, sizeof(response.headers), "Content-Type: text/plain\r\nContent-Length: %zu", response.body_size);
    }

    LOG_INFO("DELETE method response created");
    return response;
}

struct Response handle_method_other() {
    struct Response response = initialize_response();

    response.body = strdup("Method not allowed");
    if (response.body == NULL) {
        LOG_ERROR("Memory allocation failed while creating response body");
        response.body = "";
        response.body_size = 0;
        strcpy(response.status, STATUS_500_INTERNAL_SERVER_ERROR);
        snprintf(response.headers, sizeof(response.headers),
                "Content-Type: text/plain\r\nContent-Length: %zu", response.body_size);
        return response;
    }
    response.body_size = strlen(response.body);
    response.body_size = response.body != NULL ? strlen(response.body) : 0;
    strcpy(response.status, STATUS_405_METHOD_NOT_ALLOWED);
    snprintf(response.headers, sizeof(response.headers), "Content-Type: text/plain\r\nContent-Length: %zu", response.body_size);

    LOG_WARN("Other method response created");
    return response;
}

char* convert_struct_to_string(struct Response response) {
    size_t total_size = HTTP_STATUS_SIZE + HTTP_HEADER_SIZE + response.body_size + RESPONSE_EXTRA_BYTES;
    char* response_str = malloc(total_size);
    if (response_str == NULL) {
        LOG_ERROR("Memory for response string not allocated");
        if (response.body) free(response.body);
        return NULL;
    }

    int bytes_written = snprintf(response_str, total_size, "%s\r\n%s\r\n\r\n", response.status, response.headers);
    if (response.body && response.body_size > 0) {
        memcpy(response_str + bytes_written, response.body, response.body_size);
        response_str[bytes_written + response.body_size] = '\0';
    } else {
        response_str[bytes_written] = '\0';
    }

    if (response.body) free(response.body);

    LOG_INFO("Response struct successfully converted to string");
    return response_str;
}

int is_keep_alive(const char* headers) {  
    return strstr(headers, "Connection: keep-alive") != NULL;
}