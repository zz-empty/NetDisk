#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/config.h"

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
        fprintf(stderr, "Error: cannot open config file: %s\n", config_file);
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

    while (fgets(line, sizeof(line), file)) {
        line_num++;
        char *trimmed_line = trim(line);
        
        // 跳过空行和注释
        if (*trimmed_line == 0 || *trimmed_line == '#') {
            continue;
        }

        // 解析键值对
        char *equals = strchr(trimmed_line, '=');
        if (!equals) {
            fprintf(stderr, "Warning: invaild config line %d:%s\n", line_num, trimmed_line);
            continue;
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
        } else {
            fprintf(stderr, "Warning: Unknown config key '%s' at line %d\n", key, line_num);
        }
    }

    fclose(file);
    return 0;
}

void print_config(const ServerConfig *config) {
    printf("===Server Configuration===\n");
    printf("IP: %s\n", config->ip);
    printf("Port: %d\n", config->port);
    printf("Thread: %d\n", config->thread_num);
    printf("File Directory: %s\n", config->file_dir);
    printf("==========================\n");
}

// 验证配置的合理性
int validate_config(const ServerConfig *config) {
    if (strlen(config->ip) == 0) {
        fprintf(stderr, "Error: IP address cannot be empty\n");
        return -1;
    }

    if (config->port < 1024 || config->port > 65535) {
        fprintf(stderr, "Error: Port must be between 1024 and 65535\n");
        return -1;
    }

    // 验证线程数量
    if (config->thread_num < 1 || config->thread_num > 100) {
        fprintf(stderr, "Error: Thread num must be between 1 and 100\n");
        return -1;
    }

    if (strlen(config->file_dir) == 0) {
        fprintf(stderr, "Error: File directory cannot be empty\n");
        return -1;
    }

    struct stat st;
    if (stat(config->file_dir, &st) == -1) {
        if (mkdir(config->file_dir, 0775) == -1) {
            fprintf(stderr, "Error: cannot create directory: %s\n", config->file_dir);
            return -1;
        }

        printf("Create directory: %s\n", config->file_dir);
    }

    return 0;
}
