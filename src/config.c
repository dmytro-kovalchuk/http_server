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
#include <arpa/inet.h>
#include "../include/common.h"

static struct Config config;

static int get_value_from_config(const char* config_str, const char* field, char* output) {
    if (config_str == NULL || field == NULL || output == NULL) {
        return RET_ARGUMENT_IS_NULL;
    }

    char field_pattern[FIELD_PATTERN_SIZE];
    snprintf(field_pattern, sizeof(field_pattern), "\"%s\"", field);

    const char* field_position = strstr(config_str, field_pattern);
    if (field_position == NULL) return RET_CONFIG_PARSING_ERROR;

    const char* curr_position = strchr(field_position, ':');
    if (curr_position == NULL) return RET_CONFIG_PARSING_ERROR;

    curr_position++;
    while (isspace(*curr_position)) {
        curr_position++;
    }
    if (*curr_position == '\0') return RET_CONFIG_PARSING_ERROR;

    size_t value_length;
    if (*curr_position == '\"') {
        curr_position++;
        const char* end = strchr(curr_position, '\"');
        if (end == NULL) return RET_CONFIG_PARSING_ERROR;
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
    return RET_SUCCESS;
}

static int parse_and_set_config(char* config_str) {
    if (config_str == NULL) return RET_ARGUMENT_IS_NULL;

    char buffer[CONFIG_FIELD_BUFFER_SIZE];

    if (get_value_from_config(config_str, "ip", buffer) == RET_SUCCESS) {
        struct in_addr address;
        if (inet_pton(AF_INET, buffer, &address) == 1) {
            config.ip = ntohl(address.s_addr);
        }
    }

    if (get_value_from_config(config_str, "port", buffer) == RET_SUCCESS) {
        int port = atoi(buffer);
        if (port > 0 && port <= MAX_PORT) {
            config.port = port;
        }
    }

    if (get_value_from_config(config_str, "max_clients", buffer) == RET_SUCCESS) {
        int clients = atoi(buffer);
        if (clients > 0) {
            config.max_clients = clients;
        }   
    }

    if (get_value_from_config(config_str, "root_directory", buffer) == RET_SUCCESS) {
        strncpy(config.root_directory, buffer, sizeof(config.root_directory));  
    }

    if (get_value_from_config(config_str, "log_file", buffer) == RET_SUCCESS) {
        strncpy(config.log_file, buffer, sizeof(config.log_file));
    }

    free(config_str);
    return RET_SUCCESS;
}

static char* read_config(const char* path) {
    if (path == NULL) return NULL;

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
    fclose(file);
    return config_str;
}

static void initialize_config() {
    config.ip = DEFAULT_IP_VALUE;
    config.port = DEFAULT_PORT_VALUE;
    config.max_clients = DEFAULT_MAX_CLIENTS_VALUE;
    strncpy(config.root_directory, DEFAULT_ROOT_DIR_VALUE, sizeof(config.root_directory));
    strncpy(config.log_file, DEFAULT_LOG_FILE_VALUE, sizeof(config.log_file));
}

enum ReturnCode load_config(const char* path) {
    initialize_config();
    char* config_str = read_config(path);
    return parse_and_set_config(config_str);
}

const struct Config* get_config() {
    return &config;
}