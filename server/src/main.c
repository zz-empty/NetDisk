#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    printf("NetDisk Server Starting...\n");

    if (2 != argc) {
        fprintf(stderr, "Usage: %s <config_file_path>\n", argv[0]);
        exit(1);
    }

    // 后续添加配置，网络初始化等
    printf("config file: %s\n", argv[1]);
    return 0;
}
