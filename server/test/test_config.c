// server/test/test_config.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/config.h"

void test_basic_config(void) {
    printf("=== Test 1: Basic Configuration Loading ===\n");
    
    ServerConfig config;
    if (load_config("../conf/server.conf", &config) == 0) {
        printf("✓ Configuration loaded successfully\n");
        print_config(&config);
    } else {
        printf("✗ Failed to load configuration: %s\n", get_config_error());
        return;
    }
    
    if (validate_config(&config) == 0) {
        printf("✓ Configuration validation passed\n");
    } else {
        printf("✗ Configuration validation failed: %s\n", get_config_error());
    }
    printf("\n");
}

void test_invalid_config(const char* filename, const char* test_name) {
    printf("=== Test: %s ===\n", test_name);
    
    ServerConfig config;
    if (load_config(filename, &config) != 0) {
        printf("✓ Expected failure: %s\n", get_config_error());
    } else if (validate_config(&config) != 0) {
        printf("✓ Validation caught error: %s\n", get_config_error());
    } else {
        printf("✗ Should have failed but didn't\n");
    }
    printf("\n");
}

void test_reload_config(void) {
    printf("=== Test: Configuration Reload ===\n");
    
    ServerConfig config;
    if (load_config("../conf/server.conf", &config) != 0) {
        printf("✗ Cannot load initial config: %s\n", get_config_error());
        return;
    }
    
    int original_port = config.port;
    config.port = 9999; // 修改配置
    
    if (reload_config("../conf/server.conf", &config) == 0) {
        printf("✓ Configuration reloaded successfully\n");
        if (config.port == original_port) {
            printf("✓ Config was properly reset\n");
        } else {
            printf("✗ Config not properly reset\n");
        }
    } else {
        printf("✗ Failed to reload config: %s\n", get_config_error());
    }
    printf("\n");
}

int main() {
    printf("Starting Configuration Module Tests...\n\n");
    
    test_basic_config();
    test_reload_config();
    
    // 测试各种错误情况
    test_invalid_config("nonexistent.conf", "Non-existent File");
    
    printf("All tests completed!\n");
    return 0;
}

