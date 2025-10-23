#include "../include/http_communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/file_storage.h"
#include "../include/logger.h"

#define STATUS_200_OK           "HTTP/1.1 200 OK"
#define STATUS_201_CREATED      "HTTP/1.1 201 Created"
#define STATUS_404_NOT_FOUND    "HTTP/1.1 404 Not Found"
#define STATUS_405_METHOD_NOT_ALLOWED "HTTP/1.1 405 Method Not Allowed"

struct Request parse_request(const char* raw_request) {
    struct Request request = {0};
    request.body = NULL;
    request.body_size = 0;

    sscanf(raw_request, "%15s %511s %31s", request.method, request.path, request.version);

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
            request.body = malloc(extra_bytes);
            if (request.body != NULL) {
                memcpy(request.body, after_header, extra_bytes);
                request.body_size = extra_bytes;
            }
        } else {
            strncpy(request.headers, raw_request, sizeof(request.headers)-1);
            request.headers[sizeof(request.headers) - 1] = '\0';
        }
    }

    log_message(INFO, "Raw request parsed successfully");
    return request;
}

char* create_response(struct Request request) {
    struct Response response;
    memset(&response, 0, sizeof(response));
    response.body = NULL;
    response.body_size = 0;

    if (strcmp(request.method, "GET") == 0) {
        response = handle_method_get(request);
    } else if (strcmp(request.method, "POST") == 0) {
        response = handle_method_post();
    } else if (strcmp(request.method, "DELETE") == 0) {
        response = handle_method_delete(request);
    } else {
        response = handle_method_other();
    }

    return convert_struct_to_string(response);
}

size_t parse_content_length(const char* headers) {
    if (!headers) {
        log_message(INFO, "No headers detected");
        return 0;
    }

    const char* curr_line = headers;
    while (curr_line != NULL && *curr_line != '\0') {
        const char* line_end = strstr(curr_line, "\r\n");
        if (line_end == NULL) {
            line_end = curr_line + strlen(curr_line);
        }

        if (strncasecmp(curr_line, "Content-Length:", 15) == 0) {
            const char* val = curr_line + 15;
            while (*val == ' ' || *val == '\t') val++;
            log_message(INFO, "Found and parsed 'Content-Length' header");
            return (size_t)strtoul(val, NULL, 10);
        }

        if (*line_end == '\0') break;

        curr_line = line_end + 2;
    }

    log_message(INFO, "No 'Content-Length' header detected");
    return 0;
}

struct Response handle_method_get(struct Request request) {
    struct Response response;

    if (!is_file_exists(request.path)) {
        log_message(WARN, "GET method: file not found");
        strcpy(response.status, STATUS_404_NOT_FOUND);
        strcpy(response.headers, "Content-Type: text/plain\r\nContent-Length: 10");
        response.body = strdup("Not Found");
        response.body_size = response.body != NULL ? strlen(response.body) : 0;
    } else {
        log_message(INFO, "GET method: file exists");
        strcpy(response.status, STATUS_200_OK);
        char header[128];
        snprintf(header, sizeof(header), "Content-Type: application/octet-stream\r\nContent-Length: %zu", get_file_size(request.path));
        strncpy(response.headers, header, sizeof(response.headers) - 1);
    }

    log_message(INFO, "GET method response created");
    return response;
}

struct Response handle_method_post() {
    struct Response response;

    log_message(INFO, "POST method: file created");
    strcpy(response.status, STATUS_201_CREATED);
    strcpy(response.headers, "Content-Type: text/plain\r\nContent-Length: 17");
    response.body = strdup("File created.\n");
    response.body_size = response.body != NULL ? strlen(response.body) : 0;

    log_message(INFO, "POST method response created");
    return response;
}

struct Response handle_method_delete(struct Request request) {
    struct Response response;

    if (delete_file(request.path) == 0) {
        log_message(INFO, "DELETE method: file deleted");
        strcpy(response.status, STATUS_200_OK);
        strcpy(response.headers, "Content-Type: text/plain\r\nContent-Length: 15");
        response.body = strdup("File deleted.\n");
        response.body_size = response.body != NULL ? strlen(response.body) : 0;
    } else {
        log_message(WARN, "DELETE method: file doesn't exists");
        strcpy(response.status, STATUS_404_NOT_FOUND);
        strcpy(response.headers, "Content-Type: text/plain\r\nContent-Length: 10");
        response.body = strdup("Not Found");
        response.body_size = response.body != NULL ? strlen(response.body) : 0;
    }

    log_message(INFO, "DELETE method response created");
    return response;
}

struct Response handle_method_other() {
    struct Response response;

    strcpy(response.status, STATUS_405_METHOD_NOT_ALLOWED);
    strcpy(response.headers, "Content-Type: text/plain\r\nContent-Length: 18");
    response.body = strdup("Method not allowed");
    response.body_size = response.body != NULL ? strlen(response.body) : 0;

    log_message(WARN, "Other method response created");
    return response;
}

char* convert_struct_to_string(struct Response response) {
    size_t total_size = 256 + (response.body_size);
    char* response_str = malloc(total_size);
    if (response_str == NULL) {
        log_message(ERROR, "Memory for response string not allocated");
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

    log_message(INFO, "Response struct successfully converted to string");
    return response_str;
}