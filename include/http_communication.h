#ifndef HTTP_COMMUNICATION_H
#define HTTP_COMMUNICATION_H

typedef unsigned long size_t;

struct Request {
    char method[16];
    char path[512];
    char version[32];
    char headers[8192];
    char* body;
    size_t body_size;
};

struct Response {
    char status[64];
    char headers[256];
    char* body;
    size_t body_size;
};

struct Request parse_request(const char* raw_request);

char* create_response(struct Request request);

size_t parse_content_length(const char* headers);

#endif