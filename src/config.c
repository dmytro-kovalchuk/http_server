/**
    * @file: config.c
    * @author: Dmytro Kovalchuk
    *
    * This file contains definitions of functions related to
    * configuration management for the server application.
    *
    * It handles reading, parsing, and loading of configuration
    * data from a specified file into a structured format used
    * throughout the program. If no configuration file is found
    * or certain fields are missing, default values are applied.
*/

#include "../include/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

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
    if (size <= 0) {
        fclose(file);
        return NULL;
    }
    rewind(file);

    char* config_str = malloc(size + 1);
    if (config_str == NULL) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(config_str, 1, size, file);
    if (bytes_read <= 0) {
        fclose(file);
        free(config_str);
        return NULL;
    }

    config_str[size] = '\0';
    return config_str;
}

struct Config parse_config(const char* config_str) {
    struct Config parsed_config;
    strncpy(parsed_config.ip, DEFAULT_IP_VALUE, sizeof(parsed_config.ip));
    parsed_config.port = DEFAULT_PORT_VALUE;
    parsed_config.max_clients = DEFAULT_MAX_CLIENTS_VALUE;
    strncpy(parsed_config.root_directory, DEFAULT_ROOT_DIR_VALUE, sizeof(parsed_config.root_directory));
    strncpy(parsed_config.log_file, DEFAULT_LOG_FILE_VALUE, sizeof(parsed_config.log_file));

    if (config_str == NULL) return parsed_config;

    char buffer[256];

    if (get_value_from_config(config_str, "ip", buffer) == 0) {
        strncpy(parsed_config.ip, buffer, sizeof(parsed_config.ip));
    }

    if (get_value_from_config(config_str, "port", buffer) == 0) {
        int port = atoi(buffer);
        if (port > 0 && port <= 65535) {
            parsed_config.port = port;
        }
    }

    if (get_value_from_config(config_str, "max_clients", buffer) == 0) {
        int clients = atoi(buffer);
        if (clients > 0) {
            parsed_config.max_clients = clients;
        }   
    }

    if (get_value_from_config(config_str, "root_directory", buffer) == 0) {
        strncpy(parsed_config.root_directory, buffer, sizeof(parsed_config.root_directory));  
    }

    if (get_value_from_config(config_str, "log_file", buffer) == 0) {
        strncpy(parsed_config.log_file, buffer, sizeof(parsed_config.log_file));
    }

    return parsed_config;
}

int get_value_from_config(const char* config_str, const char* field, char* output) {
    if (config_str == NULL || field == NULL || output == NULL) {
        return -1;
    }

    char field_pattern[64];
    snprintf(field_pattern, sizeof(field_pattern), "\"%s\"", field);

    const char* field_position = strstr(config_str, field_pattern);
    if (field_position == NULL) return -1;

    const char* curr_position = strchr(field_position, ':');
    if (curr_position == NULL) return -1;

    curr_position++;
    while (isspace(*curr_position)) {
        curr_position++;
    }
    if (*curr_position == '\0') return -1;

    size_t value_length;
    if (*curr_position == '\"') {
        curr_position++;
        const char* end = strchr(curr_position, '\"');
        if (end == NULL) return -1;
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
    return 0;
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