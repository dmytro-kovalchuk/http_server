#ifndef COMMON_H
#define COMMON_H

// === HTTP Request and Response structs ===
#define HTTP_VERSION_SIZE 32
#define HTTP_HEADER_FIELD_SIZE 256
#define HTTP_STATUS_SIZE 64

// === Config ===
#define DEFAULT_IP_VALUE INADDR_LOOPBACK
#define DEFAULT_PORT_VALUE 8080
#define DEFAULT_MAX_CLIENTS_COUNT 5
#define DEFAULT_ROOT_DIR "./storage"
#define DEFAULT_LOG_FILENAME "log.txt"

#define FIELD_PATTERN_SIZE 64
#define CONFIG_FIELD_BUFFER_SIZE 256
#define MAX_PORT 65535
#define METHOD_STR_LEN 16

// === HTTP statuses ===
#define STATUS_200_OK                       "HTTP/1.1 200 OK"
#define STATUS_201_CREATED                  "HTTP/1.1 201 Created"
#define STATUS_404_NOT_FOUND                "HTTP/1.1 404 Not Found"
#define STATUS_405_METHOD_NOT_ALLOWED       "HTTP/1.1 405 Method Not Allowed"
#define STATUS_500_INTERNAL_SERVER_ERROR    "HTTP/1.1 500 Internal Server Error"

// === Other ===
#define MAX_PATH_LEN 256
#define RESPONSE_EXTRA_BYTES 10

// === Return codes ===
enum ReturnCode {
    RET_SUCCESS = 0,
    RET_ERROR = -1,
    RET_ARGUMENT_IS_NULL = -2,
    RET_CONFIG_PARSING_ERROR = -3,
    RET_FILE_NOT_OPENED = -4

};

#endif // COMMON_H