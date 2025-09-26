#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/command.h"
#include "../include/command_parser.h"

void test_command_creation() {
    printf("=== Test 1: Command Result Creation ===\n");

    command_result_t* success  = command_result_create(true, "Test success");
    if (success && success->success) {
        printf("✓ Success result created: %s\n", success->message);
    } else {
        printf("✗ Failed to create success result\n");
    }
    command_result_destroy(success);

    
    command_result_t* failure  = command_result_create(false, "Test failure");
    if (failure && !failure->success) {
        printf("✓ failure result created: %s\n", failure->message);
    } else {
        printf("✗ Failed to create failure result\n");
    }
    command_result_destroy(failure);
    printf("\n");
}

void test_context_management() {
    printf("=== Test 2: Command Context Management ===\n");

    command_context_t* context = command_context_create(1, "/tmp");
    if (context) {
        printf("✓ Context created: client_id=%d, root=%s, current=%s\n",
               context->client_id, context->root_dir, context->current_dir);

        command_context_destroy(context);
        printf("✓ Context destroyed\n");
    } else {
        printf("✗ Failed to create context\n");
    }
    printf("\n");
}

void test_command_parser() {
    printf("=== Test 3: Command Parsing ===\n");

    const char *test_input = "nd_list /home/user";
    parser_command_t* cmd = command_parser(test_input, strlen(test_input));

    if (cmd && cmd->command && cmd->arguments) {
        printf("✓ Command parsed: %s %s\n", cmd->command, cmd->arguments);
        
        if (strcmp(cmd->command, "nd_list") == 0 && 
            strcmp(cmd->arguments, "/home/user") == 0) {
            printf("✓ Command parsing correct\n");
        }
    } else {
        printf("✗ Failed to parse command\n");
    }

    command_free(cmd);
    printf("\n");
}

void test_basic_command() {
    printf("=== Test 4: Basic Command Execution ===\n");

    command_context_t* context = command_context_create(1, ".");
    command_result_t* result;

#if 1
    // 测试 ng_getcwd
    result = nd_getcwd(context, NULL);
    if (result && result->success) {
        printf("✓ nd_getcwd: %s\n", result->message);
    } else {
        printf("✗ nd_getcwd failed\n");
    }
    command_result_destroy(result);
#endif

    // 测试 nd_list
    result = nd_list(context, ".");
    if (result) {
        if (result->success) {
            printf("✓ nd_list succeeded");
            if (result->data_size > 0) {
                printf(" (%zu bytes of data)", result->data_size);
            }
            printf("\n");
        } else {
            printf("✗ nd_list failed: %s\n", result->message);
        }
    }

    command_result_destroy(result);

    // 创建测试目录
    mkdir("test_dir", 0755);

    // 测试 nd_chdir
    result = nd_chdir(context, "test_dir"); 
    if (result && result->success) {
        printf("✓ nd_chdir: %s\n", result->message);
    }
    command_result_destroy(result);

    // 返回原目录
    nd_chdir(context, "..");

    // 测试 nd_remove
    result = nd_remove(context, "test_dir");
    if (result && result->success) {
        printf("✓ nd_remove: %s\n", result->message);
    } else {
        // 如果删除失败，尝试清理
        rmdir("test_dir");
    }
    command_result_destroy(result);

    command_context_destroy(context);
    printf("\n");
} 

void test_command_registry() {
    printf("=== Test 5: Command Registry ===\n");

    commands_init();

    // 测试获取已知命令
    command_handler_t handler = commands_get_handler("nd_list");
    if (handler) {
        printf("✓ Found handler for nd_list\n");
    } else {
        printf("✗ Cannot find handler for nd_list\n");
    }

    // 测试获取位置命令
    handler = commands_get_handler("noexistent");
    if (!handler) {
        printf("✓ Correctly returned NULL for nonexistent command\n");
    } else {
        printf("✗ Should return NULL for nonexistent command\n");
    }

    commands_cleanup();
    printf("\n");
}

int main() {
    printf("Start Command Module Tests...\n");

    test_command_creation();
    test_context_management();
    test_command_parser();
    test_basic_command();
    test_command_registry();

    printf("All command module tests completed!\n");
    return 0;
}
