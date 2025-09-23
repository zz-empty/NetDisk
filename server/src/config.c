#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "../include/config.h"

static char config_error[2056];
const char *get_config_error(void) {
    return config_error;
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
int load_config(const char *config_file, ServerConfig *config) {
    // 打开文件
    FILE *file = fopen(config_file, "r");
    if (!file) {
        /* fprintf(stderr, "Error: cannot open config file: %s\n", config_file); */
        snprintf(config_error, sizeof(config_error), "Cannot open config file: %s, error: %s", config_file, strerror(errno));
        return -1;
    }

    // 解析每一行
    char line[MAX_CONFIG_LINE];
    int line_num = 0;

    // 设置默认值
    strncpy(config->ip, "127.0.0.1", sizeof(config->ip));
    config->port = 8080;
    config->thread_num = 10;
    strncpy(config->file_dir, "./server_files", sizeof(config->file_dir));
    config->max_connections = 100;
    config->timeout = 300;

    while (fgets(line, sizeof(line), file)) {
        line_num++;
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

        if (strcmp(key, "IP") == 0) {
            strncpy(config->ip, value, sizeof(config->ip) - 1);
        } else if (strcmp(key, "PORT") == 0) {
            config->port = atoi(value);
        } else if (strcmp(key, "THREAD_NUM") == 0) {
            config->thread_num = atoi(value);
        } else if (strcmp(key, "FILE_DIR") == 0) {
            strncpy(config->file_dir, value, sizeof(config->file_dir) - 1);
        } else if (strcmp(key, "MAX_CONNECTIONS") == 0) {
            config->max_connections = atoi(value);
        } else if (strcmp(key, "TIMEOUT") == 0) {
            config->timeout = atoi(value);
        }
    }

    fclose(file);
    return 0;
}

// 重新加载配置（保留部分状态）
int reload_config(const char *config_file, ServerConfig *config) {
    ServerConfig new_config;
    
    if (load_config(config_file, &new_config) != 0) {
        return -1;
    }

    if (validate_config(&new_config) != 0) {
        return -1;
    }

    // 更新配置
    *config = new_config;
    return 0;
}


void print_config(const ServerConfig *config) {
    printf("===Server Configuration===\n");
    printf("IP: %s\n", config->ip);
    printf("Port: %d\n", config->port);
    printf("Thread: %d\n", config->thread_num);
    printf("File Directory: %s\n", config->file_dir);
    printf("Max Connections: %d\n", config->max_connections);
    printf("Timeout: %d seconds\n", config->timeout);
    printf("==========================\n");
}

// 验证配置的合理性
int validate_config(const ServerConfig *config) {
    if (strlen(config->ip) == 0) {
        /* fprintf(stderr, "Error: IP address cannot be empty\n"); */
        snprintf(config_error, sizeof(config_error), "IP address cannot be empty");
        return -1;
    }

    // 简单的ip验证
    int dots = 0;
    for (const char *p = config->ip; *p; p++) {
        if (*p == '.') {
            dots++;
        }
        else if (*p < '0' || *p > '9') {
            snprintf(config_error, sizeof(config_error), "Invaild IP address format");
            return -1;
        }
    }

    if (dots != 3) {
        snprintf(config_error, sizeof(config_error), "Invaild IP address format");
        return -1;
    }


    if (config->port < 1024 || config->port > 65535) {
        snprintf(config_error, sizeof(config_error), "Port must be between 1024 and 65535, got %d", config->port);
        return -1;
    }

    // 验证线程数量
    if (config->thread_num < 1 || config->thread_num > 100) {
        snprintf(config_error, sizeof(config_error), "Thread num must be between 1 and 100, got %d",config->thread_num);
        return -1;
    }

    // 验证最大连接数
    if (config->max_connections <= 0 || config->max_connections > 10000) {
        snprintf(config_error, sizeof(config_error),
                "Max connections must be between 1 and 10000, got %d", config->max_connections);
        return -1;
    }

    // 验证超时时间
    if (config->timeout <= 0) {
        snprintf(config_error, sizeof(config_error),
                "Timeout must be positive, got %d", config->timeout);
        return -1;
    }

    if (strlen(config->file_dir) == 0) {
        fprintf(stderr, "Error: File directory cannot be empty\n");
        return -1;
    }

    struct stat st;
    if (stat(config->file_dir, &st) == -1) {
        if (mkdir(config->file_dir, 0775) == -1) {
            snprintf(config_error, sizeof(config_error),
                    "Cannot create directory: %s, error: %s", config->file_dir, strerror(errno));
            return -1;
        }

        printf("Create directory: %s\n", config->file_dir);
    } else if (!S_ISDIR(st.st_mode)) {
        snprintf(config_error, sizeof(config_error), 
                 "File directory path exists but is not a directory: %s", config->file_dir);
        return -1;
    }

    config_error[0] = '\0';     // 清空
    return 0;
}
