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
#include <pthread.h>
#include "../include/config.h"
#include "../include/common.h"

static FILE* log_file;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

enum ReturnCode initialize_logger() {
    const struct Config* config = get_config();
    log_file = fopen(config->log_file, "a");
    if (log_file == NULL) {
        return RET_ERROR;
    }
    return RET_SUCCESS;
}

void log_message(enum Level level, const char* message) {
    if (message == NULL) return;

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

    pthread_mutex_lock(&log_mutex);
    fprintf(log_file, "[%s] [%s] %s\n", curr_time_str, level_str, message);
    fflush(log_file);
    pthread_mutex_unlock(&log_mutex);
}

void deinitialize_logger() {
    pthread_mutex_lock(&log_mutex);
    fclose(log_file);
    log_file = NULL;
    pthread_mutex_unlock(&log_mutex);
    pthread_mutex_destroy(&log_mutex);
}