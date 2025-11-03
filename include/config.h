/**
    * @file: config.h
    * @author: Dmytro Kovalchuk
    *
    * This file contains declarations of functions and structures
    * related to configuration handling for the server application.
    *
    * The configuration includes parameters such as IP address, port,
    * maximum number of clients, root directory, and log file path.
*/

#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

/**
    * @struct Config
    * @brief Structure representing the server configuration parameters.
*/
struct Config {
    unsigned int ip;              /**< The IP address of the server. */
    unsigned int port;            /**< The port number on which the server listens. */
    unsigned int max_clients;     /**< Maximum number of clients the server can handle concurrently. */
    char root_directory[MAX_PATH_LEN];     /**< Path to the root directory of the server's file storage. */
    char log_file[MAX_PATH_LEN];           /**< Path to the server's log file. */
};

/**
    * Loads configuration from a specified file path.
    *
    * @param[in] path The path to the configuration file.
    *
    * This function reads, parses, and loads configuration values
    * into a global configuration structure used by the application.
    *
    * @note If the configuration file is missing or invalid,
    * default values are applied.
*/
void load_config(const char* path);

/**
    * Retrieves the const pointer to the const loaded configuration.
    *
    * @return Returns a pointer to const pointer to const struct Config.
*/
const struct Config* get_config();

#endif // CONFIG_H