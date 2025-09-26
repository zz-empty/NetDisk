#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// 命令执行结果结构
typedef struct {
    bool success;       // 是否成功
    int error_code;     // 错误码
    char *message;      // 结果消息
    char *data;         // 返回数据
    size_t data_size;   // 数据大小
} command_result_t;

// 命令上下文（客户端会话消息）
typedef struct {
    int client_id;              // 客户端ID
    char current_dir[2056];     // 当前工作目录
    char root_dir[2056];        // 根目录（限制访问范围）
    char username[32];          // 用户名
    bool authenticated;         // 是否已认证
} command_context_t;

// 命令处理函数类型
typedef command_result_t* (*command_handler_t)(command_context_t *context, const char *args);

// 命令注册结构
typedef struct {
    const char *name;           // 命令名称
    command_handler_t handler;  // 处理函数
    const char *description;    // 命令描述
    bool require_auth;          // 是否需要认证
} command_entry_t;


// 函数声明
command_result_t* nd_chdir(command_context_t* context, const char* args);
command_result_t* nd_list(command_context_t* context, const char* args);
command_result_t* nd_getcwd(command_context_t* context, const char* args);
command_result_t* nd_upload(command_context_t* context, const char* args);
command_result_t* nd_download(command_context_t* context, const char* args);
command_result_t* nd_remove(command_context_t* context, const char* args);
command_result_t* nd_mkdir(command_context_t* context, const char* args);

// 命令管理函数
void commands_init();
command_handler_t commands_get_handler(const char *command_name);
void commands_cleanup();

// 工具函数
command_result_t* command_result_create(bool success, const char* message);
command_result_t* command_result_create_with_data(bool success, const char* message, const char *data, size_t data_size);
void command_result_destroy(command_result_t *result);

command_context_t* command_context_create(int client_id, const char* root_dir);
void command_context_destroy(command_context_t *context);

#endif  // COMMAND_H
