#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <time.h>
#include <fcntl.h>
#include "../include/network.h"
#include "../include/protocol.h"
#include "../include/threadpool.h"
#include "../include/command.h"
#include "../include/command_parser.h"

#define MAX_EVENTS 64
#define BUFFER_SIZE 4096

// 客户端连接处理任务
typedef struct {
    network_server_t *server;
    client_connection_t *client;
} client_task_t;

// 创建网络服务器
network_server_t *network_server_create(server_config_t *config, threadpool_t *pool) {
    if (!config || !pool) {
        return NULL;
    }

    network_server_t *server = (network_server_t*)malloc(sizeof(network_server_t));
    if (!server) {
        return NULL;
    }

    memset(server, 0, sizeof(network_server_t));
    server->port = config->server_port;
    server->config = config;
    server->thread_pool = pool;
    server->max_clients = config->max_connections;
    server->next_client_fd = 1;

    // 分配客户端连接数组
    server->clients = (client_connection_t**)calloc(server->max_clients, sizeof(client_connection_t*));
    if (!server->clients) {
        free(server);
        return NULL;
    }

    return server;
}

// 创建监听套接字
static int create_listen_socket(int port) {
    int listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listen_fd == -1) {
        return -1;
    }

    // 设置套接字选项
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(listen_fd);
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(listen_fd);
        return -1;
    }

    if (listen(listen_fd, SOMAXCONN) < 0) {
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}

// 处理客户端连接
static void handle_client_connection(network_server_t *server, client_connection_t *client) {
    (void)server;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // 接受数据
    network_error_t result = network_receive_data(client->fd, buffer, sizeof(buffer), &bytes_received);

    if (result == NETWORK_SUCCESS && bytes_received > 0) {
        // 更新最后活动时间
        client->last_activity = time(NULL);

        // 解析和执行命令
        parser_command_t* cmd = command_parser(buffer, bytes_received);
        if (cmd) {
            // 创建命令上下文（简化版）
            command_context_t* context = command_context_create(client->client_id, server->config->data_directory);
            if (context) {
                // 执行命令
                command_result_t* cmd_result = command_execute(context, cmd);

                if (cmd_result) {
                    // 发送命令结果
                    if (cmd_result->message) {
                        network_send_data(client->client_id, cmd_result->message, strlen(cmd_result->message));
                    } 

                    // 如果有数据，也发送
                    if (cmd_result->data && cmd_result->data_size > 0) {
                        network_send_data(client->client_id, cmd_result->data, cmd_result->data_size);
                    }

                    command_result_destroy(cmd_result);
                }

                command_context_destroy(context);
            }

            command_free(cmd);
        }
    }
    else if (result == NETWORK_CONNECTION_CLOSED) {
        printf("Client %d disconnected\n", client->client_id);
        // 标记连接关闭，将在主循环中清理
    }
}

// 客户端处理任务函数
static void client_task_function(void *arg) {
    client_task_t *task = (client_task_t*)arg;
    if (!task || !task->server || !task->client) {
        free(task);
        return;
    }

    handle_client_connection(task->server, task->client);
    free(task);
}

// 接受新的客户端连接
static network_error_t accept_new_connection(network_server_t *server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(server->listen_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return NETWORK_SUCCESS;     // 没有新连接，不是错误
        }
        return NETWORK_ACCEPT_ERROR;
    }

    // 设置客户端套接字为非阻塞
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    // 创建客户端连接
    client_connection_t *client = client_connection_create(client_fd, client_addr);
    if (!client) {
        close(client_fd);
        return NETWORK_SOCKET_ERROR;
    }

    client->client_id = server->next_client_fd++;

    // 添加到客户端数组中（简化版，应该换成更高效的数据结构）
    for (int i = 0; i < server->max_clients; i++) {
        if (server->clients[i] == NULL) {
            server->clients[i] = client;
            break;
        }
    }

    // 添加到epoll监听
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;   // 边沿触发
    event.data.fd = client_fd;

    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
        client_connection_destroy(client);
        return NETWORK_EPOLL_ERROR;
    }

    printf("New client connected: %s:%d (ID: %d)\n", 
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client->client_id);

    return NETWORK_SUCCESS;
}

// 启动网络服务器
network_error_t network_server_start(network_server_t *server) {
    if (!server) {
        return NETWORK_SOCKET_ERROR;
    }

    // 创建监听套接字
    server->listen_fd = create_listen_socket(server->port);
    if (server->listen_fd == -1) {
        return NETWORK_SOCKET_ERROR;
    }

    // 创建epoll实例
    server->epoll_fd = epoll_create1(0);
    if (server->epoll_fd == -1) {
        return NETWORK_EPOLL_ERROR;
    }

    // 添加监听套接字到epoll
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server->listen_fd;

    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->listen_fd, &event) == -1) {
        close(server->listen_fd);
        close(server->epoll_fd);
        return NETWORK_EPOLL_ERROR;
    }

    server->running = true;
    printf("Network server started on port %d\n", server->port);

    // 主事件循环
    struct epoll_event events[MAX_EVENTS];
    while (server->running) {
        int num_events = epoll_wait(server->epoll_fd, events, MAX_EVENTS, 100);     // 1s超时
        if (num_events == -1) {
            if (errno == EAGAIN) {
                continue;   // 被信号中断，继续
            }
            break;      // 其他错误，退出循环·
        }

        // 处理响应的event
        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == server->listen_fd) {
                // 新连接请求
                accept_new_connection(server);
            } else {
                // 客户端数据可读
                client_connection_t *client = (client_connection_t*)events[i].data.ptr;

                // 创建任务并提交到线程池
                client_task_t *task = (client_task_t*)malloc(sizeof(client_task_t));
                if (task) {
                    task->client = client;
                    task->server = server;
                    threadpool_add(server->thread_pool, client_task_function, task);
                }
            }
        }

        // 清理断开的连接（简化版本）
        // 实际应该使用更复杂的心跳和超时机制
    }

    return NETWORK_SUCCESS;
}

// 关闭客户端连接
network_error_t network_server_stop(network_server_t *server) {
    if (!server) {
        return NETWORK_SOCKET_ERROR;
    }

    server->running = false;

    // 关闭所有客户端连接
    for (int i = 0; i < server->max_clients; i++) {
        if (server->clients[i]) {
            client_connection_destroy(server->clients[i]);
            server->clients[i] = NULL;
        }
    }

    // 关闭套接字和epoll
    if (server->listen_fd != -1) {
        close(server->listen_fd);
        server->listen_fd = -1;
    }

    if (server->epoll_fd != -1) {
        close(server->epoll_fd);
        server->epoll_fd = -1;
    }

    printf("Network server stopped!\n");
    return NETWORK_SUCCESS;
}

// 销毁网络服务器
void network_server_destroy(network_server_t *server) {
    if (!server) {
        return;
    } 

    network_server_stop(server);

    if (server->clients) {
        free(server->clients);
    }

    free(server);
}

// 创建客户端连接
client_connection_t *client_connection_create(int fd, struct sockaddr_in addr) {
    if (fd < 0) {
        return NULL;
    }
    client_connection_t *client = (client_connection_t*)malloc(sizeof(client_connection_t));
    if (!client) {
        return NULL;
    }

    memset(client, 0, sizeof(client_connection_t));
    client->fd = fd;
    client->addr = addr;
    client->connect_time = time(NULL);
    client->last_activity = time(NULL);
    client->authenticated = false;
    memset(client->username, 0, sizeof(client->username));

    return client;
}

// 销毁客户端连接
void client_connection_destroy(client_connection_t *client) {
    if (!client) {
        return;
    }

    if (client->fd != -1) {
        close(client->fd);
    }

    free(client);
}

// 发送数据到客户端
network_error_t client_connection_send(client_connection_t *client, const char *data, size_t length) {
    if (!client || !data) {
        return NETWORK_SOCKET_ERROR;
    }

    return network_send_data(client->fd, data, length);
}

// 发送数据
network_error_t network_send_data(int fd, const char *data, size_t length) {
    if (fd == -1 || !data) {
        return NETWORK_SOCKET_ERROR;
    }

    ssize_t total_send = 0;

    while ((size_t)total_send < length) {
        ssize_t sent = send(fd, data + total_send, length - total_send, MSG_NOSIGNAL);

        if (sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 缓冲区满，稍后重试
                usleep(1000);   // 1ms
                continue;
            }
            return NETWORK_SOCKET_ERROR;
        }

        total_send += sent;
    }

    return NETWORK_SUCCESS;
}

// 接受数据
network_error_t network_receive_data(int fd, char *buffer, size_t buffer_size, ssize_t *received) {
    if (fd == -1 || !buffer || !received) {
        return NETWORK_SOCKET_ERROR;
    }

    *received = recv(fd, buffer, buffer_size - 1, 0);    // 保留一个字节给字符串终止符

    if (*received == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            *received = 0;
            return NETWORK_SUCCESS;     // 没有数据可读，不是错误
        }
        return NETWORK_SOCKET_ERROR;
    } else if (*received == 0) {
        return NETWORK_CONNECTION_CLOSED;   // 连接关闭
    }

    buffer[*received] = '\0';
    return NETWORK_SUCCESS;
}

// 获取错误字符串
const char *network_get_error_string(network_error_t error) {
    switch (error) {
    case NETWORK_SUCCESS: return "Success";
    case NETWORK_SOCKET_ERROR: return "Socket error";
    case NETWORK_BIND_ERROR: return "Bind error";
    case NETWORK_LISTEN_ERROR: return "Listen error";
    case NETWORK_ACCEPT_ERROR: return "Accept error";
    case NETWORK_EPOLL_ERROR: return "Epoll error";
    case NETWORK_CONNECTION_CLOSED: return "Connection closed";
    case NETWORK_TIMEOUT: return "Timeout";
    case NETWORK_BUFFER_FULL: return "Buffer full";
    case NETWORK_PROTOCOL_ERROR: return "Protocol error";
    default: return "Unknown error";
    }
}
