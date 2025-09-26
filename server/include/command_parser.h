#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "command.h"

// 解析后的命令结构
typedef struct {
    char* command;      // 命令名称
    char* arguments;    // 命令参数
    size_t data_size;   // 附加数据大小
    char* data;         // 附加数据（用于上传等）
} parser_command_t;

// 函数声明
parser_command_t* command_parser(const char* input, size_t input_length);
void command_free(parser_command_t* cmd);
command_result_t* command_execute(command_context_t* context, const parser_command_t* cmd);

#endif  // COMMAND_PARSER_H
