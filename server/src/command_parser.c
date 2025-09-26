#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/command_parser.h"

// 解析命令输入
parser_command_t* command_parser(const char* input, size_t input_length) {
    if (!input || input_length == 0) {
        return NULL;
    } 

    parser_command_t* cmd = (parser_command_t*)malloc(sizeof(parser_command_t));
    if (!cmd) return NULL;

    memset(cmd, 0, sizeof(parser_command_t));

    // 简单的空格分割: 命令 参数
    const char* space = strchr(input, ' ');
    if (space) {
        // 有参数
        size_t cmd_len = space - input;
        cmd->command = (char*)malloc(cmd_len + 1);
        strncpy(cmd->command, input, cmd_len);
        cmd->command[cmd_len] = '\0';

        // 参数是剩余部分, 去除前导空格
        const char* args_start = space + 1;
        while (*args_start && isspace(*args_start)) args_start++;

        if (*args_start) {
            size_t args_len = input_length - (args_start - input);
            cmd->arguments = (char*)malloc(args_len + 1);
            strncpy(cmd->arguments, args_start, args_len);
            cmd->arguments[args_len] = '\0';
        }
    } else {
        // 没有参数
        cmd->command = (char*)malloc(input_length + 1);
        strncpy(cmd->command, input, input_length);
        cmd->command[input_length] = '\0';
    }

    return cmd;
}

// 释放解析的命令
void command_free(parser_command_t* cmd) {
    if (!cmd) return;

    if (cmd->command) free(cmd->command);
    if (cmd->arguments) free(cmd->arguments);
    if (cmd->data) free(cmd->data);
    free(cmd);
}

// 执行解析后的命令
command_result_t* command_execute(command_context_t* context, const parser_command_t* cmd) {
    if (!context || !cmd || !cmd->command) {
        return command_result_create(false, "Invalid command");
    }

    // 获取命令处理函数
    command_handler_t handler = commands_get_handler(cmd->command);
    if (!handler) {
        char message[256];
        snprintf(message, sizeof(message), "Unknown command: %s\n", cmd->command);
        return command_result_create(false, message);
    }

    // 检查认证要求
    // 这里简化处理，实际应该检查命令是否需要认证
    
    // 执行命令
    return handler(context, cmd->arguments);
}
