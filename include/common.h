#ifndef COMMON_H
#define COMMON_H

// === HTTP Request and Response structs ===
#define HTTP_VERSION_SIZE 32
#define HTTP_HEADER_SIZE 8192
#define HTTP_STATUS_SIZE 64

// === Config ===
#define DEFAULT_IP_VALUE INADDR_LOOPBACK
#define DEFAULT_PORT_VALUE 8080
#define DEFAULT_MAX_CLIENTS_VALUE 5
#define DEFAULT_ROOT_DIR_VALUE "./storage"
#define DEFAULT_LOG_FILE_VALUE "log.txt"

#define FIELD_PATTERN_SIZE 64
#define CONFIG_FIELD_BUFFER_SIZE 256
#define MAX_PORT 65535
#define METHOD_STR_LEN 16

// === HTTP statuses ===
#define STATUS_200_OK                   "HTTP/1.1 200 OK"
#define STATUS_201_CREATED              "HTTP/1.1 201 Created"
#define STATUS_404_NOT_FOUND            "HTTP/1.1 404 Not Found"
#define STATUS_405_METHOD_NOT_ALLOWED   "HTTP/1.1 405 Method Not Allowed"

// === Other ===
#define MAX_PATH_LEN 256
#define RESPONSE_EXTRA_BYTES 10

#endif // COMMON_H