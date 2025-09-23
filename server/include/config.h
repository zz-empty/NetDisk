#ifndef CONFIG_H
#define CONFIG_H

#define MAX_CONFIG_LINE 256
#define MAX_FILE_PATH 1024

// 服务器配置结构体
typedef struct {
    char ip[16];
    int port;
    int thread_num;
    char file_dir[MAX_FILE_PATH];
} ServerConfig;

// 函数声明
int load_config(const char *config_file, ServerConfig *config);
void print_config(const ServerConfig *config);
int validate_config(const ServerConfig *config);

#endif  // CONFIG_H
