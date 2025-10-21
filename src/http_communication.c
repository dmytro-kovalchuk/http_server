#include "../include/http_communication.h"

#include <stdio.h>
#include <string.h>
#include "../include/file_storage.h"

struct Request parse_request(const char* raw_request) {
    struct Request request;

    sscanf(raw_request, "%7s %255s %15s", request.method, request.path, request.version);

    const char* header_end = strstr(raw_request, "\r\n\r\n");
    while (header_end != NULL) {
        size_t header_size = header_end - raw_request;
        strncpy(request.headers, raw_request, header_size);

        strncpy(request.body, header_end + 4, sizeof(request.body) - 1);
    }

    return request;
}

char* create_response(int client_socket, struct Request request) {
    struct Response response;

    if (strcmp(request.method, "GET") == 0) {
        if (is_file_exists(request.path, "r") == 0) {
            snprintf(response.status, sizeof(response.status), "HTTP/1.1 404 Not Found\r\n");
            snprintf(response.body, sizeof(response.body), "File Not Found");
        } else {
            snprintf(response.status, sizeof(response.status), "HTTP/1.1 200 OK\r\n");
            send_file(client_socket, request.path);
        }
    } else if (strcmp(request.method, "POST") == 0) {
        if (is_file_exists(request.path, "w") == 0) {
            snprintf(response.status, sizeof(response.status), "HTTP/1.1 500 Internal Error\r\n");
            snprintf(response.body, sizeof(response.body), "Cannot Create File");
        } else {
            snprintf(response.status, sizeof(response.status), "HTTP/1.1 200 OK\r\n");
            snprintf(response.body, sizeof(response.body), "File Created Successfully");
            receive_file(client_socket, request.path, parse_content_length(request.headers));
        }
    } else if (strcmp(request.method, "DELETE") == 0) {
        if (delete_file(request.path) == 0) {
            snprintf(response.status, sizeof(response.status), "HTTP/1.1 200 OK\r\n");
            snprintf(response.body, sizeof(response.body), "File Deleted Successfully");
        } else {
            snprintf(response.status, sizeof(response.status), "HTTP/1.1 404 Not Found\r\n");
            snprintf(response.body, sizeof(response.body), "File Not Found");
        }
    } 
}

size_t parse_content_length(const char* headers) {
    const char* cl = strstr(headers, "Content-Length:");
    if (cl == NULL) return 0;
    return strtoul(cl + 15, NULL, 10);
}