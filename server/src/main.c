// server/src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "../include/config.h"
#include "../include/threadpool.h"

// 全局变量
static server_config_t *g_config = NULL;
static threadpool_t *g_thread_pool = NULL;

// 信号处理
void signal_handler(int sig) {
    printf("\nReceive signal %d, shutting down...\n", sig);

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


    // 示例：添加一些任务到线程池
    for (int i = 0; i < 5; i++) {
        int *task_id = (int*)malloc(sizeof(int));
        *task_id = i;

        if (threadpool_add(g_thread_pool, sample_task, task_id) != THREADPOOL_SUCCESS) {
            free(task_id);
            fprintf(stderr, "Failed to add task %d to thread pool\n", i);
        }
    }

    printf("Server is running. Press Ctrl+C to stop.\n");

    // 主循环（简化版）
    while (1) {
        /* sleep(1); */
        // 即将添加网络监听模块
    }

    return 0;
}
