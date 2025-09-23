#include <stdio.h>
#include <stdlib.h>
#include "../include/config.h"

int main(int argc, char **argv) {
    printf("NetDisk Server Starting...\n");

    if (2 != argc) {
        fprintf(stderr, "Usage: %s <config_file_path>\n", argv[0]);
        exit(1);
    }

    // 加载配置信息
    ServerConfig config;
    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Configuration loaded failed\n");
        exit(1);
    }

    if (validate_config(&config) == -1) {
        fprintf(stderr, "Configuration validated failed\n");
        exit(1);
    }

    // 显示配置信息
    print_config(&config);
    printf("Server Configuration loaded successfully!\n");

    // 后续添加网络初始化模块
    printf("Ready to initlialize network components...\n");

    return 0;
}
