#include "../include/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../include/logger.h"

#define DEFAULT_IP_VALUE "127.0.0.1"
#define DEFAULT_PORT_VALUE 8080
#define DEFAULT_MAX_CLIENTS_VALUE 5
#define DEFAULT_ROOT_DIR_VALUE "./storage"
#define DEFAULT_LOG_FILE_VALUE "log.txt"

static struct Config config;

void load_config(const char* path) {
    char* config_str = read_config(path);
    config = parse_config(config_str);
    free(config_str);
}

char* read_config(const char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* config_str = malloc(size + 1);
    fread(config_str, 1, size, file);
    config_str[size] = '\0';

    return config_str;
}

struct Config parse_config(const char* config_str) {
    struct Config parsed_config;
    
    if (config_str == NULL) {
        strncpy(parsed_config.ip, DEFAULT_IP_VALUE, sizeof(parsed_config.ip));
        parsed_config.port = DEFAULT_PORT_VALUE;
        parsed_config.max_clients = DEFAULT_MAX_CLIENTS_VALUE;
        strncpy(parsed_config.root_directory, DEFAULT_ROOT_DIR_VALUE, sizeof(parsed_config.root_directory));
        strncpy(parsed_config.log_file, DEFAULT_LOG_FILE_VALUE, sizeof(parsed_config.log_file));

        return parsed_config;
    }

    char buffer[256];

    get_value_from_config(config_str, "ip", buffer);
    strncpy(parsed_config.ip, buffer, sizeof(parsed_config.ip));

    get_value_from_config(config_str, "port", buffer);
    parsed_config.port = atoi(buffer);

    get_value_from_config(config_str, "max_clients", buffer);
    parsed_config.max_clients = atoi(buffer);

    get_value_from_config(config_str, "root_directory", buffer);
    strncpy(parsed_config.root_directory, buffer, sizeof(parsed_config.root_directory));

    get_value_from_config(config_str, "log_file", buffer);
    strncpy(parsed_config.log_file, buffer, sizeof(parsed_config.log_file));

    return parsed_config;
}

void get_value_from_config(const char* config_str, const char* field, char* output) {
    char field_pattern[64];
    snprintf(field_pattern, sizeof(field_pattern), "\"%s\"", field);

    const char* field_position = strstr(config_str, field_pattern);
    const char* curr_position = strchr(field_position, ':');

    curr_position++;
    while (isspace(*curr_position)) {
        curr_position++;
    }

    size_t value_length;
    if (*curr_position == '\"') {
        curr_position++;
        const char* end = strchr(curr_position, '\"');
        value_length = end - curr_position;
    } else {
        const char* end = curr_position;
        while (*end && *end != ',' && *end != '}') {
            end++;
        }
        value_length = end - curr_position;
    }

    strncpy(output, curr_position, value_length);
    output[value_length] = '\0';
}

char* get_ip_from_config() {
    return config.ip;
}

unsigned int get_port_from_config() {
    return config.port;
}

unsigned int get_max_clients_from_config() {
    return config.max_clients;
}

char* get_root_dir_from_config() {
    return config.root_directory;
}

char* get_log_file_from_config() {
    return config.log_file;
}