// server/src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "../include/config.h"
#include "../include/threadpool.h"
#include "../include/network.h"

// 全局变量
static server_config_t *g_config = NULL;
static threadpool_t *g_thread_pool = NULL;
static network_server_t *g_network_server = NULL;

// 信号处理
void signal_handler(int sig) {
    printf("\nReceive signal %d, shutting down...\n", sig);

    if (g_network_server) {
        network_server_stop(g_network_server);
        network_server_destroy(g_network_server);
        g_network_server = NULL;
    }

    if (g_thread_pool) {
        threadpool_destroy(g_thread_pool, 1);   // 优雅关闭
    }

    if (g_config) {
        config_destroy(g_config);
    }

    exit(0);
}

// 示例函数
void sample_task(void *arg) {
    int task_id = *(int*)arg;
    printf("Processing task %d in thread pool\n", task_id);
    free(arg);
}

int main(int argc, char **argv) {
    if (2 != argc) {
        fprintf(stderr, "Usage: %s <config_file_path>\n", argv[0]);
        exit(1);
    }

    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    printf("NetDisk Server Starting...\n");


    // 加载配置信息
    const char *config_file = argc > 1 ? argv[1] : "server.conf";
    config_error_t config_result = config_load_from_file(config_file, &g_config);

    if (config_result != CONFIG_SUCCESS) {
        fprintf(stderr, "Failed to load configuration: %s\n", config_get_error_string(config_result));
        return 1;
    }

    // 显示配置信息
    config_print(g_config);

    // 创建线程池
    g_thread_pool = threadpool_create(config_get_thread_pool_size(g_config), config_get_max_connections(g_config));  // 队列大小
    if (!g_thread_pool) {
        fprintf(stderr, "Failed to create thread pool\n");
        config_destroy(g_config);
        return 1;
    }

    printf("Thread pool created with %d threads\n", config_get_thread_pool_size(g_config));

    // 创建网络服务器
    g_network_server = network_server_create(g_config, g_thread_pool);
    if (!g_network_server) {
        fprintf(stderr, "Failed to create network server\n");
        threadpool_destroy(g_thread_pool, 0);
        config_destroy(g_config);
        return 1;
    }

    printf("Network server created!\n");

    // 启动网络服务器   (会阻塞)
    network_error_t network_result = network_server_start(g_network_server);
    if (network_result != NETWORK_SUCCESS) {
        fprintf(stderr, "Failed to start network server: %s\n", network_get_error_string(network_result));
        network_server_destroy(g_network_server);
        threadpool_destroy(g_thread_pool, 0);
        config_destroy(g_config);
        return 1;
    }

    // 清理资源（正常情况下不会运行到这里
    network_server_destroy(g_network_server);
    threadpool_destroy(g_thread_pool, 0);
    config_destroy(g_config);

    return 0;
}
