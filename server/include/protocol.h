#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// 协议版本
#define NETWORK_PROTOCOL_VERSION 1

// 命令类型
typedef enum {
    CMD_AUTH = 0x01,        // 认证命令
    CMD_LS = 0x02,          // 列出文件
    CMD_CD = 0x03,          // 改变目录
    CMD_GET = 0x04,         // 下载文件
    CMD_PUT = 0x05,         // 上传文件
    CMD_RM = 0x06,          // 删除文件
    CMD_MKDIR = 0x07,       // 创建目录
    CMD_PWD = 0x08,         // 当前目录
    CMD_QUIT = 0x09,        // 退出连接
    CMD_RESPONSE = 0x80,    // 响应命令（最高位为1）
} command_type_t;

// 响应状态码
typedef enum {
    RESPONSE_SUCCESS = 0,
    RESPONSE_ERROR = 1,
    RESPONSE_AUTH_REQUIRED = 2,
    RESPONSE_PERMISSION_DENIED = 3,
    RESPONSE_FILE_NOT_FOUND = 4,
    RESPONSE_INVALID_COMMAND = 0,
} response_status_t;

// 协议头文件（固定16字节）, GCC编译器的特有语法，取消结构体的内存对齐
typedef struct __attribute__((packed)) {
    uint8_t version;        // 协议版本
    uint8_t command;        // 命令类型
    uint16_t flags;         // 标志位
    uint32_t sequence;      // 序列号
    uint32_t data_length;   // 数据长度
    uint32_t checksum;      // 校验和
} protocol_header_t;

// 认证请求
typedef struct __attribute__((packed)) {
    char username[32];      // 用户名
    char password[32];      // 密码
} auth_request_t;

// 文件信息结构
typedef struct __attribute__((packed)) {
    char filename[256];     // 文件名 
    uint64_t file_size;     // 文件大小
    uint32_t permissions;   // 权限
    uint64_t modify_time;   // 修改时间
    uint8_t file_type;      // 文件类型（0-文件，1-目录）
} file_info_t;

// 响应结构
typedef struct __attribute__((packed)) {
    uint8_t status;         // 响应状态
    uint32_t data_length;   // 数据长度
} response_header_t;

// 函数声明
bool protocol_validate_header(const protocol_header_t *header);
uint32_t protocol_calculate_checksum(const char *data, size_t length);
bool protocol_validate_checksum(const protocol_header_t *header, const char *data);

protocol_header_t protocol_create_header(uint8_t command, uint32_t data_length);
response_header_t protocol_create_response(uint8_t status, uint32_t data_length);

char *protocol_serialize_file_info(const file_info_t *file_info, size_t *serialize_size);
bool protocol_deserialize_file_info(const char *data, file_info_t *file_info);

#endif // PROTOCOL_H
