// server/test/test_config.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/config.h"

void test_config_creation() {
    printf("=== Test 1: Configuration Creation ===\n");
    server_config_t *config = config_create_default();
    if (!config) {
        printf("✗ Failed to create default configuration\n");
        return;
    }

    config_print(config);
    config_destroy(config);
    printf("✓ Default configuration created successfully\n\n");
}

void test_config_loading() {
    printf("=== Test 2: Configuration Loading ===\n");

    // 创建测试配置文件
    FILE* test_file = fopen("test_config.conf", "w");
    if (!test_file) {
        printf("✗ Failed to create test configuration file\n");
        return;
    }

    fprintf(test_file, "# Test Configuration File\n");
    fprintf(test_file, "server_ip=127.0.0.1\n");
    fprintf(test_file, "server_port=9090\n");
    fprintf(test_file, "thread_pool_size=20\n");
    fprintf(test_file, "max_connections=200\n");
    fprintf(test_file, "request_timeout=60\n");
    fprintf(test_file, "data_directory=/tmp/netdisk\n");
    fprintf(test_file, "max_file_size=500\n");
    fprintf(test_file, "buffer_size=128\n");
    fprintf(test_file, "enable_logging=true\n");
    fprintf(test_file, "log_file=/tmp/netdisk.log\n");
    fclose(test_file);
    
    // 加载配置
    server_config_t *config = NULL;
    config_error_t result = config_load_from_file("test_config.conf", &config);

    if (result == CONFIG_SUCCESS && config) {
        config_print(config);

        // 验证加载的指
        if (strcmp(config->server_ip, "127.0.0.1") == 0 &&
            config->server_port == 9090 &&
            config->thread_pool_size == 20) {
            printf("✓ Configuration loaded and validated successfully\n");
        } else {
            printf("✗ Configuration values mismatch\n");
        }

        config_destroy(config);
    } else {
        printf("✗ Failed to load configuration: %s\n", config_get_error_string(result));
    }

    // 清理测试文件
    unlink("test_config.conf");
    printf("\n");
}

void test_config_validation() {
    printf("=== Test 3: Configuration Validation ===\n");

    server_config_t* config = config_create_default();
    if (!config) {
        printf("✗ Failed to create configuration for validation test\n");
        return;
    }

    // 测试有效配置
    config_error_t result = config_validate(config);
    if (result == CONFIG_SUCCESS) {
        printf("✓ Valid configuration passed validation\n");
    } else {
        printf("✗ Valid configuration failed validation: %s\n", config_get_error_string(result));
    }

    // 测试无效端口
    config->server_port = 0;
    result = config_validate(config);
    if (result == CONFIG_VALIDATION_ERROR) {
        printf("✓ Invalid port correctly detected\n");
    } else {
        printf("✗ Invalid port not detected\n");
    }

    config_destroy(config);
    printf("\n");
}

void test_error_handling() {
    printf("=== Test 4: Error Handling ===\n");

    // 测试不存在的文件
    server_config_t* config = NULL;
    config_error_t result = config_load_from_file("nonexistent.conf", &config);
    if (result == CONFIG_FILE_NOT_FOUND) {
        printf("✓ Non-existent file correctly handled\n");
    } else {
        printf("✗ Non-existent file not handled correctly\n");
    }

    // 测试空指针
    result = config_load_from_file(NULL, &config);
    if (result == CONFIG_PARSE_ERROR) {
        printf("✓ NULL filename correctly handled\n");
    } else {
        printf("✗ NULL filename not handled correctly\n");
    }

    printf("\n");
}

int main() {
    printf("Starting Configuration Module Tests (POSIX Style)...\n\n");

    test_config_creation();
    test_config_loading();
    test_config_validation();
    test_error_handling();

    printf("All configuration module tests completed!\n");
    return 0;
}

