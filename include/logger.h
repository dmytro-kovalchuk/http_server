#ifndef LOGGER_H
#define LOGGER_H

#define LOG_FILE_NAME "log.txt"

enum Level {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

void log_message(enum Level level, const char* message);

#endif