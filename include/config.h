#ifndef CONFIG_H
#define CONFIG_H

struct Config {
    char ip[32];
    unsigned int port;
    unsigned int max_clients;
    char root_directory[256];
    char log_file[256];
};

void load_config(const char* path);

char* read_config(const char* path);

struct Config parse_config(const char* config_str);

void get_value_from_config(const char* config_str, const char* field, char* output);

char* get_ip_from_config();

unsigned int get_port_from_config();

unsigned int get_max_clients_from_config();

char* get_root_dir_from_config();

char* get_log_file_from_config();

#endif