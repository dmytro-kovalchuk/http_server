/**
    * @file: http_header.h
    * @author: Dmytro Kovalchuk
    *
    * This file contains declarations for managing HTTP headers
    * within the server. It provides functionality to add, retrieve,
    * check, and free HTTP headers in a structured HeaderList.
    *
    * The HeaderList structure allows dynamic storage of multiple
    * headers and ensures proper memory management.
*/

#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#include "http_messages.h"

/**
    * Adds a header key-value pair to the given HeaderList.
    *
    * @param[in,out] list Pointer to the HeaderList to which the header will be added.
    * @param[in] key The name of the HTTP header.
    * @param[in] value The value of the HTTP header.
*/
void add_header(struct HeaderList* list, const char* key, const char* value);

/**
    * Adds a formatted header to the HeaderList using printf-style formatting.
    *
    * @param[in,out] list Pointer to the HeaderList to which the header will be added.
    * @param[in] key The name of the HTTP header.
    * @param[in] format The printf-style format string for the header value.
    * @param[in] ... Additional arguments used for formatting.
*/
void add_header_formatted(struct HeaderList* list, const char* key, const char* format, ...);

/**
    * Retrieves the value of a header from the HeaderList by its key.
    *
    * @param[in] list Pointer to the HeaderList to search.
    * @param[in] key The name of the header to retrieve.
    *
    * @return Returns a pointer to the header value if found, or NULL if the header does not exist.
*/
const char* get_header_value(const struct HeaderList* list, const char* key);

/**
    * Frees all memory associated with a HeaderList.
    *
    * @param[in,out] list Pointer to the HeaderList to be freed.
    *
    * @note After calling this function, the list will be empty.
*/
void free_headers(struct HeaderList* list);

#endif // HTTP_HEADER_H