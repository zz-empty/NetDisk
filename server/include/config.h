#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>
// 统一使用POSIX风格命名

typedef struct {
    char server_ip[16];         // 服务器IP地址 
    int server_port;            // 服务器端口
    int thread_pool_size;       // 线程池大小
    int max_connections;        // 最大连接数
    int request_timeout;        // 请求超时时间（秒
    char data_directory[256];    // 数据存储目录
    int max_file_size;          // 最大文件大小（MB
    int buffer_size;            // 缓冲区大小（KB
    bool enable_logging;        // 是否启动日志
    char log_file[256];         // 日志文件路径
} server_config_t;

// 错误码定义
typedef enum {
    CONFIG_SUCCESS = 0,
    CONFIG_FILE_NOT_FOUND = -1,
    CONFIG_PARSE_ERROR = -2,
    CONFIG_VALIDATION_ERROR = -3,
    CONFIG_MEMORY_ERROR = -4
} config_error_t;

// 函数声明
server_config_t *config_create_default();
config_error_t config_load_from_file(const char *filename, server_config_t **config);
config_error_t config_validate(const server_config_t *config);
void config_print(const server_config_t *config);
void config_destroy(server_config_t *config);
const char *config_get_error_string(config_error_t error);

// 便捷函数
const char *config_get_data_directory(const server_config_t *config);
int config_get_thread_pool_size(const server_config_t *config);
int config_get_max_connections(const server_config_t *config);

#endif  // CONFIG_H
