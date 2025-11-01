/**
    * @file: logger.h
    * @author: Dmytro Kovalchuk
    *
    * This file contains declarations of logging utilities used
    * throughout the server application.
    *
    * It provides functions for writing formatted log messages
    * to log file, categorized by levels such as DEBUG, INFO,
    * WARN, ERROR, and FATAL.
*/

#ifndef LOGGER_H
#define LOGGER_H

/**
    * @enum Level
    * @brief Represents the severity level of a log message.
*/
enum Level {
    DEBUG,  /**< Detailed information for debugging. */
    INFO,   /**< General informational messages. */
    WARN,   /**< Warnings indicating potential issues. */
    ERROR,  /**< Error messages for failed operations. */
    FATAL   /**< Critical errors that cause program exit. */
};

/**
    * Logs a message with the specified severity level.
    *
    * @param[in] level The severity level of the message.
    * @param[in] message The message string to log.
*/
void log_message(enum Level level, const char* message);

#endif // LOGGER_H