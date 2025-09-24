#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "../include/config.h"

// 默认配置
server_config_t *config_create_default() {
    server_config_t *config = (server_config_t*)malloc(sizeof(server_config_t));
    if (!config) {
        return NULL;
    }

    // 清空
    memset(config, 0, sizeof(server_config_t));

    // 设置默认值
    strcpy(config->server_ip, "0.0.0.0");
    config->server_port = 8080;
    config->thread_pool_size = 10;
    config->max_connections = 100;
    config->request_timeout = 30;
    strcpy(config->data_directory, "./data");
    config->max_file_size = 100;    // 100MB
    config->enable_logging = true;
    strcpy(config->log_file, "./logs/server.log");
    config->buffer_size = 64;       // 64KB

    return config;
}

// 去除首尾空白
static char *trim(char *str) {
    char *end;

    // 去除首部空白
    while (str && (*str == ' ' || *str == '\t' || *str == '\n')) {
        str++;
    }

    if (*str == 0) {    // 只有空白
        return str;
    }

    // 去除尾部空白
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\n' || *end == '\t')) {
        end--;
    }

    // 设置终止符
    *(end + 1) = 0;
    return str;
}

// 加载配置文件
config_error_t config_load_from_file(const char *filename, server_config_t **config_ptr) {
    if (!filename || !config_ptr) {
        return CONFIG_PARSE_ERROR;
    }

    // 检查文件是否存在
    if (access(filename, F_OK) != 0) {
        return CONFIG_FILE_NOT_FOUND;
    }

    // 打开文件
    FILE *file = fopen(filename, "r");
    if (!file) {
        return CONFIG_FILE_NOT_FOUND;
    }

    // 创建默认配置作为基础
    server_config_t *config = config_create_default();
    if (!config) {
        fclose(file);
        return CONFIG_MEMORY_ERROR;
    }

    // 解析每一行
    char line[256];


    while (fgets(line, sizeof(line), file)) {
        char *trimmed_line = trim(line);
        
        // 跳过空行和注释
        if (*trimmed_line == 0 || *trimmed_line == '#' || *trimmed_line == ';') {
            continue;
        }

        // 解析键值对
        char *equals = strchr(trimmed_line, '=');
        if (!equals) {
            continue;   // 跳过格式错误的行
        }

        *equals = '\0';
        char *key = trim(trimmed_line);
        char *value = trim(equals + 1);

        if (strcmp(key, "server_ip") == 0) {
            strncpy(config->server_ip, value, sizeof(config->server_ip) - 1);
        } 
        else if (strcmp(key, "server_port") == 0) {
            config->server_port = atoi(value);
        } 
        else if (strcmp(key, "thread_pool_size") == 0) {
            config->thread_pool_size = atoi(value);
        } 
        else if (strcmp(key, "data_dicectory") == 0) {
            strncpy(config->data_directory, value, sizeof(config->data_directory) - 1);
        } 
        else if (strcmp(key, "max_connections") == 0) {
            config->max_connections = atoi(value);
        } 
        else if (strcmp(key, "request_timeout") == 0) {
            config->request_timeout = atoi(value);
        }
        else if (strcmp(key, "max_file_size") == 0) {
            config->max_file_size = atoi(value);
        }
        else if (strcmp(key, "buffer_size") == 0) {
            config->buffer_size = atoi(value);
        }
        else if (strcmp(key, "enable_logging") == 0) {
            // true/1/yes为true，否则为false
            if (strcasecmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcasecmp(value, "yes") == 0) {
                config->enable_logging = true;
            } else {
                config->enable_logging = false;
            }
        }
        else if (strcmp(key, "log_file") == 0) {
            strncpy(config->log_file, value, sizeof(config->log_file) - 1);
        }
    }

    fclose(file);

    // 验证配置信息
    config_error_t validation_result = config_validate(config);
    if (validation_result != CONFIG_SUCCESS) {
        free(config);
        return validation_result;
    }

    *config_ptr = config;
    return CONFIG_SUCCESS;
}


void config_print(const server_config_t *config) {
    if (!config) {
        printf("Configuration is NULL\n");
        return;
    }

    printf("=== Server Configuration ===\n");
    printf("Server IP: %s\n", config->server_ip);
    printf("Server Port: %d\n", config->server_port);
    printf("Thread Pool Size: %d\n", config->thread_pool_size);
    printf("Max Connections: %d\n", config->max_connections);
    printf("Request Timeout: %d seconds\n", config->request_timeout);
    printf("Data Directory: %s\n", config->data_directory);
    printf("Max File Size: %d MB\n", config->max_file_size);
    printf("Buffer Size: %d KB\n", config->buffer_size);
    printf("Logging Enabled: %s\n", config->enable_logging ? "Yes" : "No");
    printf("Log File: %s\n", config->log_file);
    printf("============================\n");
}

// 验证配置的合理性
config_error_t config_validate(const server_config_t *config) {
    if (!config) {
        return CONFIG_VALIDATION_ERROR;
    }

    // 验证IP地址格式
    if (strlen(config->server_ip) == 0) {
        return CONFIG_VALIDATION_ERROR;
    }

    // 验证端口范围
    if (config->server_port < 1024 || config->server_port > 65535) {
        return CONFIG_VALIDATION_ERROR;
    }

    // 验证线程数量
    if (config->thread_pool_size < 1 || config->thread_pool_size > 100) {
        return CONFIG_VALIDATION_ERROR;
    }

    // 验证最大连接数
    if (config->max_connections < 1 || config->max_connections > 10000) {
        return CONFIG_VALIDATION_ERROR;
    }

    // 验证超时时间
    if (config->request_timeout < 1 || config->request_timeout > 300) {
        return CONFIG_VALIDATION_ERROR;
    }

    // 验证数据目录
    if (strlen(config->data_directory) == 0) {
        return CONFIG_VALIDATION_ERROR;
    }

    // 如果数据目录不存在，创建一个
    struct stat st;
    if (stat(config->data_directory, &st) == -1) {
        if (mkdir(config->data_directory, 0775) == -1) {
            return CONFIG_MEMORY_ERROR;
        }

        printf("Create directory: %s\n", config->data_directory);
    } else if (!S_ISDIR(st.st_mode)) {
        return CONFIG_VALIDATION_ERROR;
    }

    // 验证文件大小
    if (config->max_file_size < 1 || config->max_file_size > 1024) {
        return CONFIG_VALIDATION_ERROR;
    }

    // 验证缓冲区大小
    if (config->buffer_size < 1 || config->buffer_size > 1024) {
        return CONFIG_VALIDATION_ERROR;
    }

    return CONFIG_SUCCESS;
}


void config_destroy(server_config_t *config) {
    if (config) {
        free(config);
    }
}


const char *config_get_error_string(config_error_t error) {
    switch(error) {
    case CONFIG_SUCCESS:
        return "Success";
    case CONFIG_FILE_NOT_FOUND:
        return "Configuration file not found";
    case CONFIG_PARSE_ERROR:
        return "Error parsing configuration file";
    case CONFIG_VALIDATION_ERROR:
        return "Configuration validation failed";
    case CONFIG_MEMORY_ERROR:
        return "Memory allocation error";
    default:
        return "Unknown error";
    }
}


const char *config_get_data_directory(const server_config_t *config) {
    return config ? config->data_directory : "./data";
}
int config_get_thread_pool_size(const server_config_t *config) {
    return config ? config->thread_pool_size : 10;
}
int config_get_max_connections(const server_config_t *config) {
    return config ? config->max_connections : 100;
}
