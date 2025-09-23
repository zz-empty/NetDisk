// server/src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "../include/config.h"

volatile sig_atomic_t reload_config_flag = 0;
void handle_signup(int sig) {
    (void)sig;
    reload_config_flag = 1;
}

int main(int argc, char **argv) {
    printf("NetDisk Server Starting...\n");

    if (2 != argc) {
        fprintf(stderr, "Usage: %s <config_file_path>\n", argv[0]);
        exit(1);
    }

    // 后续添加配置，网络初始化等
    printf("config file: %s\n", argv[1]);
    // 设置信号处理（热重载）
    signal(SIGHUP, handle_signup);

    // 加载配置信息
    ServerConfig config;
    if (load_config(argv[1], &config) != 0) {
        fprintf(stderr, "Failed to load configuration: %s\n", get_config_error());
        exit(1);
    }

    if (validate_config(&config) != 0) {
        fprintf(stderr, "Configuration validation failed: %s\n", get_config_error());
        exit(1);
    }

    // 显示配置信息
    print_config(&config);

    printf("Server Configuration loaded successfully!\n");
    printf("Server is running... Press Ctrl+C to stop\n");

    // 主循环（简化版，后续添加网络初始化模块
    while (1) {
        // 检查是否需要热重载
        if (reload_config_flag) {
            printf("Receive SIGHUP, reloading configuration...\n");
            if (reload_config(argv[1], &config) != 0) {
                fprintf(stderr, "Failed to load configuration: %s\n", get_config_error());
            } else {
                printf("Configuration reloaded successfully!\n");
                print_config(&config);
            }
            reload_config_flag = 0;
        }

        // 后期改成select/epoll
        sleep(1);
    }

    return 0;
}
