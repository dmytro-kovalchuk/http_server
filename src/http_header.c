/**
    * @file: http_header.c
    * @author: Dmytro Kovalchuk
    *
    * This file contains implementations of functions for managing HTTP headers.
    *
    * It provides functionality to add headers, add formatted headers,
    * retrieve header values, check for header existence, and free
    * allocated memory associated with header lists.
*/

#include "../include/http_header.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void add_header(struct HeaderList* list, const char* key, const char* value) {
    list->items = realloc(list->items, sizeof(struct Header) * (list->size + 1));
    list->items[list->size].key = strdup(key);
    list->items[list->size].value = strdup(value);
    list->size++;
}

void add_header_formatted(struct HeaderList* list, const char* key, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[HTTP_HEADER_FIELD_SIZE];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    add_header(list, key, buffer);
}

const char* get_header_value(const struct HeaderList* list, const char* key) {
    if (list == NULL || key == NULL) return NULL;

    for (size_t i = 0; i < list->size; ++i) {
        if (strcasecmp(list->items[i].key, key) == 0) {
            return list->items[i].value;
        }
    }
    return NULL;
}

int has_header(const struct HeaderList* list, const char* key) {
    return get_header_value(list, key) != NULL;
}

void free_headers(struct HeaderList* list) {
    for (size_t i = 0; i < list->size; ++i) {
        free(list->items[i].key);
        free(list->items[i].value);
    }
    free(list->items);
    list->items = NULL;
    list->size = 0;
}