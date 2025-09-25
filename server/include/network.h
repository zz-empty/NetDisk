#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "config.h"
#include "threadpool.h"

// 网络错误码
typedef enum {
    NETWORK_SUCCESS = 0,
    NETWORK_SOCKET_ERROR = -1,
    NETWORK_BIND_ERROR = -2,
    NETWORK_LISTEN_ERROR = -3,
    NETWORK_ACCEPT_ERROR = -4,
    NETWORK_EPOLL_ERROR = -5,
    NETWORK_CONNECTION_CLOSED = -6,
    NETWORK_TIMEOUT = -7,
    NETWORK_BUFFER_FULL = -8,
    NETWORK_PROTOCOL_ERROR = -9
} network_error_t;

// 客户端连接结构
typedef struct {
    int fd;                     // 套接字文件描述符
    uint32_t client_id;         // 客户端ID
    struct sockaddr_in addr;    // 客户端地址
    time_t connect_time;        // 连接时间
    time_t last_activity;       // 最后活动时间
    bool authenticated;         // 是否已认证
    char username[32];          // 用户名
} client_connection_t;

// 网络服务器结构
typedef struct {
    int listen_fd;              // 监听套接字
    int epoll_fd;               // Epoll文件描述符
    int port;                   // 监听端口
    bool running;               // 服务器运行状态
    uint32_t next_client_fd;    // 下一个客户端ID
    client_connection_t **clients;  // 客户端连接数组
    int max_clients;            // 最大客户端数
    threadpool_t *thread_pool;  // 线程池引用
    server_config_t *config;    // 配置引用
} network_server_t;

// 网络数据缓冲区
typedef struct {
    char *data;     // 数据缓冲区
    size_t size;    // 缓冲区大小
    size_t read_pos;    // 读取位置
    size_t write_pos;   // 写入位置
} network_buffer_t;

// 网络相关函数声明
network_server_t *network_server_create(server_config_t *config, threadpool_t *pool);
network_error_t network_server_start(network_server_t *server);
network_error_t network_server_stop(network_server_t *server);
void network_server_destroy(network_server_t *server);

network_error_t network_send_data(int fd, const char *data, size_t length);
network_error_t network_receive_data(int fd, char *buffer, size_t buffer_size, ssize_t *received);

client_connection_t *client_connection_create(int fd, struct sockaddr_in addr);
void client_connection_destroy(client_connection_t *client);
network_error_t client_connection_send(client_connection_t *client, const char *data, size_t length);

network_buffer_t *network_buffer_create(size_t size);
void network_buffer_destroy(network_buffer_t *buffer);
network_error_t network_buffer_write(network_buffer_t *buffer, const char *data, size_t length);
network_error_t network_buffer_read(network_buffer_t *buffer, char *data, size_t length);

const char *network_get_error_string(network_error_t error);

#endif  // NETWORK_H
