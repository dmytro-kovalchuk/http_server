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
#include "../include/http_header.h"
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
    return request;
}

static struct HeaderList parse_headers(const char* raw_headers) {
    struct HeaderList list = {NULL, 0};
    if (raw_headers == NULL) return list;

    const char* line_start = raw_headers;
    const char* line_end;

    while ((line_end = strstr(line_start, "\r\n")) != NULL) {
        if (line_end == line_start) break;

        const char* colon = strchr(line_start, ':');
        if (colon && colon < line_end) {
            size_t key_len = colon - line_start;
            size_t value_len = line_end - (colon + 1);
            while (value_len > 0 && ((colon[1 + value_len - 1] == ' ') || (colon[1 + value_len - 1] == '\t'))) {
                value_len--;
            }

            list.items = realloc(list.items, sizeof(struct Header) * (list.size + 1));
            list.items[list.size].key = strndup(line_start, key_len);
            list.items[list.size].value = strndup(colon + 1, value_len);
            list.size++;
        }

        line_start = line_end + 2;
    }

    return list;
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
        char* header_buf = strndup(raw_request, header_size);
        request.headers = parse_headers(header_buf);
        free(header_buf);

        const char* after_header = header_end + 4;
        if (*after_header) {
            request.body_size = strlen(after_header);
            request.body = strndup(after_header, request.body_size);
        }
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

char* create_response(const struct Request* request) {
    struct Response response;

    switch (request->method) {
        case GET: response = handle_method_get(request); break;
        case POST: response = handle_method_post(); break;
        case DELETE: response = handle_method_delete(request); break;
        case UNKNOWN: 
        default: response = handle_method_other();
    }

    if (is_keep_alive(request->headers)) {
        add_header(&response.headers, "Connection", "keep-alive");
        add_header(&response.headers, "Keep-Alive", "timeout=5, max=100");
    } else {
        add_header(&response.headers, "Connection", "close");
    }

    return convert_struct_to_string(&response);
}

struct Response handle_method_get(const struct Request* request) {
    struct Response response = initialize_response();

    if (check_file_exists(request->path) != RET_SUCCESS) {
        LOG_WARN("GET: file not found");
        strcpy(response.status, STATUS_404_NOT_FOUND);
        response.body = strdup("Not Found");
        response.body_size = strlen(response.body);
        add_header(&response.headers, "Content-Type", "text/plain");
        add_header_formatted(&response.headers, "Content-Length", "%zu", response.body_size);
        return response;
    }

    LOG_INFO("GET: file found");
    strcpy(response.status, STATUS_200_OK);
    add_header(&response.headers, "Content-Type", "application/octet-stream");
    add_header_formatted(&response.headers, "Content-Length", "%zu", get_file_size(request->path));
    return response;
}

struct Response handle_method_post() {
    struct Response response = initialize_response();

    strcpy(response.status, STATUS_201_CREATED);
    response.body = strdup("File created.\n");
    response.body_size = strlen(response.body);
    add_header(&response.headers, "Content-Type", "text/plain");
    add_header_formatted(&response.headers, "Content-Length", "%zu", response.body_size);

    LOG_INFO("POST: file created response");
    return response;
}

struct Response handle_method_delete(const struct Request* request) {
    struct Response response = initialize_response();

    if (delete_file(request->path) == RET_SUCCESS) {
        strcpy(response.status, STATUS_200_OK);
        response.body = strdup("File deleted.\n");
    } else {
        strcpy(response.status, STATUS_404_NOT_FOUND);
        response.body = strdup("Not Found");
    }

    response.body_size = strlen(response.body);
    add_header(&response.headers, "Content-Type", "text/plain");
    add_header_formatted(&response.headers, "Content-Length", "%zu", response.body_size);

    LOG_INFO("DELETE method response created");
    return response;
}

struct Response handle_method_other() {
    struct Response response = initialize_response();

    strcpy(response.status, STATUS_405_METHOD_NOT_ALLOWED);
    response.body = strdup("Method not allowed");
    response.body_size = strlen(response.body);
    add_header(&response.headers, "Content-Type", "text/plain");
    add_header_formatted(&response.headers, "Content-Length", "%zu", response.body_size);

    LOG_WARN("Other/unsupported method handled");
    return response;
}

char* convert_struct_to_string(struct Response* response) {
    size_t total_estimated = HTTP_VERSION_SIZE + response->body_size + RESPONSE_EXTRA_BYTES;
    for (size_t i = 0; i < response->headers.size; ++i) {
        total_estimated += strlen(response->headers.items[i].key) + strlen(response->headers.items[i].value) + 4;
    }

    char* response_str = malloc(total_estimated);
    if (response_str == NULL) {
        LOG_ERROR("Failed to allocate response response_str");
        return NULL;
    }

    size_t offset = snprintf(response_str, total_estimated, "%s\r\n", response->status);
    for (size_t i = 0; i < response->headers.size; ++i) {
        offset += snprintf(response_str + offset, total_estimated - offset,
                           "%s: %s\r\n",
                           response->headers.items[i].key,
                           response->headers.items[i].value);
    }
    offset += snprintf(response_str + offset, total_estimated - offset, "\r\n");

    if (response->body && response->body_size > 0) {
        memcpy(response_str + offset, response->body, response->body_size);
        offset += response->body_size;
    }
    response_str[offset] = '\0';

    free(response->body);
    free_headers(&response->headers);
    return response_str;
}

int is_keep_alive(const struct HeaderList headers) {
    const char* connection_header = get_header_value(&headers, "Connection");
    return connection_header != NULL && strcasecmp(connection_header, "keep-alive") == 0;
}

void free_request(struct Request* request) {
    if (request == NULL) return;
    free_headers(&request->headers);
    free(request->body);
    request->body = NULL;
}