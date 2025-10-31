    /**
    * @file: logger.c
    * @author: Dmytro Kovalchuk
    *
    * This file contains the implementation of logging functionality
    * used across the server application.
    *
    * It provides a unified interface for writing log messages
    * to the configured log file, including timestamps and severity
    * levels such as DEBUG, INFO, WARN, ERROR, and FATAL.
*/

#include "../include/logger.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "../include/config.h"

void log_message(enum Level level, const char* message) {
    if (message == NULL) {
        return;
    }

    FILE* log_file = fopen(get_log_file_from_config(), "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }

    time_t curr_time;
    time(&curr_time);
    char* curr_time_str = ctime(&curr_time);
    curr_time_str[strcspn(curr_time_str, "\n")] = 0;

    char* level_str;
    switch (level) {
        case DEBUG: level_str = "DEBUG";    break;
        case INFO:  level_str = "INFO";     break;
        case WARN:  level_str = "WARN";     break;
        case ERROR: level_str = "ERROR";    break;
        case FATAL: level_str = "FATAL";    break;
        default:    level_str = "UNKNOWN";
    }

    fprintf(log_file, "[%s] [%s] %s\n", curr_time_str, level_str, message);

    fclose(log_file);
}