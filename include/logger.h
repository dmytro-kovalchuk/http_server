#ifndef LOGGER_H
#define LOGGER_H

enum Level {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

void log_message(enum Level level, const char* message);

#endif