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
    * Initializes logger by opening log file.
    *
    * @return Returns 0 on success or error code if log file opening fails.
*/
enum ReturnCode initialize_logger();

/**
    * Logs a message with the specified severity level.
    *
    * @param[in] level The severity level of the message.
    * @param[in] message The message string to log.
*/
void log_message(enum Level level, const char* message);

/**
    * Deinitializes logger by closing log file.
*/
void deinitialize_logger();

#define LOG_DEBUG(msg)  log_message(DEBUG, (msg))
#define LOG_INFO(msg)   log_message(INFO, (msg))
#define LOG_WARN(msg)   log_message(WARN, (msg))
#define LOG_ERROR(msg)  log_message(ERROR, (msg))
#define LOG_FATAL(msg)  log_message(FATAL, (msg))

#endif // LOGGER_H