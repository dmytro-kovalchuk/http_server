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

/**
    * @struct Config
    * @brief Structure representing the server configuration parameters.
*/
struct Config {
    char ip[32];                  /**< The IP address of the server. */
    unsigned int port;            /**< The port number on which the server listens. */
    unsigned int max_clients;     /**< Maximum number of clients the server can handle concurrently. */
    char root_directory[256];     /**< Path to the root directory of the server's file storage. */
    char log_file[256];           /**< Path to the server's log file. */
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
    * Reads the contents of a configuration file.
    *
    * @param[in] path The path to the configuration file.
    *
    * @return Returns a dynamically allocated string containing
    * the file contents, or NULL if reading fails.
    *
    * @note The caller is responsible for freeing the returned string.
*/
char* read_config(const char* path);

/**
    * Parses the configuration string and extracts key-value pairs.
    *
    * @param[in] config_str The raw configuration string.
    *
    * @return Returns a struct Config containing the parsed values.
    * If any fields are missing or invalid, default values are applied.
*/
struct Config parse_config(const char* config_str);

/**
    * Retrieves the value of a specific field from the configuration string.
    *
    * @param[in] config_str The raw configuration string.
    * @param[in] field The name of the configuration field to extract.
    * @param[out] output Buffer where the extracted value will be stored.
    *
    * @return Returns 0 if the value was successfully retrieved, or -1 if failed.
*/
int get_value_from_config(const char* config_str, const char* field, char* output);

/**
    * Retrieves the IP address from the loaded configuration.
    *
    * @return Returns a pointer to the IP address string.
*/
char* get_ip_from_config();

/**
    * Retrieves the port number from the loaded configuration.
    *
    * @return Returns the configured port number.
*/
unsigned int get_port_from_config();

/**
    * Retrieves the maximum client count from the loaded configuration.
    *
    * @return Returns the maximum number of clients.
*/
unsigned int get_max_clients_from_config();

/**
    * Retrieves the root directory path from the loaded configuration.
    *
    * @return Returns a pointer to the root directory path string.
*/
char* get_root_dir_from_config();

/**
    * Retrieves the log file path from the loaded configuration.
    *
    * @return Returns a pointer to the log file path string.
*/
char* get_log_file_from_config();

#endif // CONFIG_H