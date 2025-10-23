#include "../include/http_communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/file_storage.h"

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
            if (request.body) {
                memcpy(request.body, after_header, extra_bytes);
                request.body_size = extra_bytes;
            }
        } else {
            strncpy(request.headers, raw_request, sizeof(request.headers)-1);
            request.headers[sizeof(request.headers) - 1] = '\0';
        }
    }

    return request;
}

char* create_response(struct Request request) {
    struct Response response;
    memset(&response, 0, sizeof(response));
    response.body = NULL;
    response.body_size = 0;

    if (strcmp(request.method, "GET") == 0) {
        if (!is_file_exists(request.path)) {
            strcpy(response.status, "HTTP/1.1 404 Not Found");
            strcpy(response.headers, "Content-Type: text/plain\r\nContent-Length: 10");
            response.body = strdup("Not Found");
            response.body_size = response.body != NULL ? strlen(response.body) : 0;
        } else {
            strcpy(response.status, "HTTP/1.1 200 OK");
            char header[128];
            snprintf(header, sizeof(header), "Content-Type: application/octet-stream\r\nContent-Length: %zu", get_file_size(request.path));
            strncpy(response.headers, header, sizeof(response.headers) - 1);
        }
    } else if (strcmp(request.method, "POST") == 0) {
        strcpy(response.status, "HTTP/1.1 201 Created");
        strcpy(response.headers, "Content-Type: text/plain\r\nContent-Length: 17");
        response.body = strdup("File created.\n");
        response.body_size = response.body != NULL ? strlen(response.body) : 0;
    } else if (strcmp(request.method, "DELETE") == 0) {
        if (delete_file(request.path) == 0) {
            strcpy(response.status, "HTTP/1.1 200 OK");
            strcpy(response.headers, "Content-Type: text/plain\r\nContent-Length: 15");
            response.body = strdup("File deleted.\n");
            response.body_size = response.body != NULL ? strlen(response.body) : 0;
        } else {
            strcpy(response.status, "HTTP/1.1 404 Not Found");
            strcpy(response.headers, "Content-Type: text/plain\r\nContent-Length: 10");
            response.body = strdup("Not Found");
            response.body_size = response.body != NULL ? strlen(response.body) : 0;
        }
    } else {
        strcpy(response.status, "HTTP/1.1 405 Method Not Allowed");
        strcpy(response.headers, "Content-Type: text/plain\r\nContent-Length: 18");
        response.body = strdup("Method not allowed");
        response.body_size = response.body != NULL ? strlen(response.body) : 0;
    }

    size_t total_size = 256 + (response.body_size);
    char* response_str = malloc(total_size);
    if (response_str == NULL) {
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
    return response_str;
}

size_t parse_content_length(const char* headers) {
    if (!headers) return 0;

    const char* curr_line = headers;
    while (curr_line != NULL && *curr_line != '\0') {
        const char* line_end = strstr(curr_line, "\r\n");
        if (line_end == NULL) {
            line_end = curr_line + strlen(curr_line);
        }

        if (strncasecmp(curr_line, "Content-Length:", 15) == 0) {
            const char* val = curr_line + 15;
            while (*val == ' ' || *val == '\t') val++;
            return (size_t)strtoul(val, NULL, 10);
        }

        if (*line_end == '\0') break;

        curr_line = line_end + 2;
    }

    return 0;
}