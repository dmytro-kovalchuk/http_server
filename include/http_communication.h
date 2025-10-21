#ifndef HTTP_COMMUNICATION_H
#define HTTP_COMMUNICATION_H

struct Request {
    char method[8];
    char path[256];
    char version[16];
    char headers[512];
    char body[2048];
};

struct Response {
    char status[64];
    char body[1024];
};

struct Request parse_request(const char* raw_request);

char* create_response(int client_socket, struct Request request);

size_t parse_content_length(const char* headers);

#endif