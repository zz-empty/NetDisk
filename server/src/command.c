#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include "../include/command.h"

// 命令注册表
static command_entry_t command_table[] = {
    {"nd_chdir", nd_chdir, "Change current directory", true},
    {"nd_list", nd_list, "List directory contents", true},
    {"nd_getcwd", nd_getcwd, "Get current working directory", true},
    {"nd_upload", nd_upload, "Upload file to server", true},
    {"nd_download", nd_download, "Download file from server", true},
    {"nd_remove", nd_remove, "Remove file or directory", true},
    {"nd_mkdir", nd_mkdir, "Create directory", true},
    {NULL, NULL, NULL, false}   // 结束标记
};

// 初始化命令系统
void commands_init() {
    // 目前还没有要初始化的内容，保留接口   
}

// 根据命令获取处理函数
command_handler_t commands_get_handler(const char *command_name) {
    for (int i = 0; command_table[i].name != NULL; i++) {
        if (strcmp(command_table[i].name, command_name) == 0) {
            return command_table[i].handler;
        }
    }
    return NULL;
}

// 清理命令系统
void commands_cleanup() {
    // 保留接口，暂无清理
}

// 创建命令结果
command_result_t* command_result_create(bool success, const char* message) {
    command_result_t* result = (command_result_t*)malloc(sizeof(command_result_t));
    if (!result) return NULL;

    result->success = success;
    result->error_code = success ? 0 : errno;
    result->data = NULL;
    result->data_size = 0;

    if (message) {
        result->message = strdup(message);
    } else {
        result->message = strdup(success ? "OK" : "ERROR");
    }

    return result;
}

// 创建带数据的命令结果
command_result_t* command_result_create_with_data(bool success, const char* message, const char *data, size_t data_size) {
    command_result_t *result = command_result_create(success, message);
    if (!result) return NULL;

    if (data && data_size > 0) {
        result->data = (char*)malloc(data_size);
        if (result->data) {
            memcpy(result->data, data, data_size);
            result->data_size = data_size;
        }
    }
    
    return result;
}

// 销毁命令结果
void command_result_destroy(command_result_t *result) {
    if (!result) return;

    if (result->message) free(result->message);
    if (result->data) free(result->data);
    free(result);
}

// 创建命令上下文
command_context_t* command_context_create(int client_id, const char* root_dir) {
    command_context_t *context = (command_context_t*)malloc(sizeof(command_context_t));
    if (!context) return NULL;

    context->client_id = client_id;
    context->authenticated = false;
    memset(context->username, 0, sizeof(context->username));

    // 设置根目录和当前目录
    if (root_dir) {
        strncpy(context->root_dir, root_dir, sizeof(context->root_dir) - 1);
        strncpy(context->current_dir, root_dir, sizeof(context->current_dir) - 1);
    } else {
        strcpy(context->root_dir, ".");
        strcpy(context->current_dir, ".");
    }

    return context;
}

// 销毁命令上下文
void command_context_destroy(command_context_t *context) {
    if (!context) free(context);
}

// 安全检查，确保路径在根目录内
static bool is_path_safe(const command_context_t* context, const char *path) {
    char resolved_path[1024];
    char resolved_root[1024];

    if (!realpath(context->root_dir, resolved_root)) {
        return false;
    }

    if (!realpath(path, resolved_path)) {
        return false;
    }

    // 检查解析后的路径是否在根目录中
    return strncmp(resolved_path, resolved_root, strlen(context->root_dir)) == 0;
}

// nd_chdir 改变当前工作目录
command_result_t* nd_chdir(command_context_t* context, const char* args) {
    if (!context || !args) {
        return command_result_create(false, "Invaild arguments");
    }

    // 安全检查
    if (!is_path_safe(context, args)) {
        return command_result_create(false, "Path traversal not allowed");
    }

    // 使用系统调用chdir
    if (chdir(args) == 0) {
        // 更新当前目录
        if (getcwd(context->current_dir, sizeof(context->current_dir)) != NULL) {
            return command_result_create(true, context->current_dir);
        } else {
            return command_result_create(false, "Change directory but cannot get path");
        }
    } else {
        return command_result_create(false, strerror(errno));
    }
}

// 列出目录内容
command_result_t* nd_list(command_context_t* context, const char* args) {
    if (!context) {
        return command_result_create(false, "Invalid context");
    }

    const char* dir_path = args ? args : context->current_dir;

    // 安全检查
    if (!is_path_safe(context, dir_path)) {
        return command_result_create(false, "Path traversal not allowed");
    }

    // 使用系统调用 opendir/readdir
    DIR *dir = opendir(dir_path);
    if (!dir) {
        return command_result_create(false, strerror(errno));
    }

    // 构建目录列表
    char buffer[4096] = {0};
    size_t pos = 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // 跳过. ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 获取文件信息
        struct stat stat_buf;
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (stat(full_path, &stat_buf) == 0) {
            char line[512];
            if (S_ISDIR(stat_buf.st_mode)) {
                snprintf(line, sizeof(line), "drwxr-xr-x 1 user user %8ld %s/\n",
                         stat_buf.st_size, entry->d_name);
            } else {
                snprintf(line, sizeof(line), "-rw-r--r-- 1 user user %8ld %s\n",
                         stat_buf.st_size, entry->d_name);
            }

            // 检查缓冲区是否足够
            if (pos + strlen(line) < sizeof(buffer) - 1) {
                strcpy(buffer + pos, line);
                pos += strlen(line);
            } else {
                break;  // 缓冲区满
            }
        }
    }

    closedir(dir);

    if (pos > 0) {
        return command_result_create_with_data(true, "Directory listing", buffer, pos);
    } else {
        return command_result_create(false, "Empty Directory");
    }
}

// 获取当前工作目录
command_result_t* nd_getcwd(command_context_t* context, const char* args) {
    if (!context) {
        return command_result_create(false, "Invalid context");
    }

    (void)args; // 未使用参数

    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return command_result_create(true, cwd);
    } else {
        return command_result_create(false, strerror(errno));
    }
}

// 上传文件（简化版，需要处理文件数据）``
command_result_t* nd_upload(command_context_t* context, const char* args) {
    if (!context || !args) {
        return command_result_create(false, "Invalid arguments");
    }

    // 占位实现，实际的实现要在协议层处理
    char message[256];
    snprintf(message, sizeof(message), "Upload command received for: %s\n", args);
    return command_result_create(true, message);
}

// 下载文件（简化版，实际需要发送文件数据）
command_result_t* nd_download(command_context_t* context, const char* args) {
    if (!context || !args) {
        return command_result_create(false, "Invalid arguments");
    }

    char full_path[4096];
    snprintf(full_path, sizeof(full_path), "%s/%s", context->current_dir, args);

    // 安全检查
    if (!is_path_safe(context, full_path)) {
        return command_result_create(false, "Path traversal not allowed");
    }

    // 检查文件是否存在且是普通文件
    struct stat stat_buf;
    if (stat(full_path, &stat_buf) != 0) {
        return command_result_create(false, strerror(errno));
    }

    if (!S_ISREG(stat_buf.st_mode)) {
        return command_result_create(false, "Not a reguler file");
    }

    // 使用系统调用读取文件
    int fd = open(full_path, O_RDONLY);
    if (fd == -1) {
        return command_result_create(false, strerror(errno));
    }

    // 读取文件内容（简化版，应该流式传输
    char *file_data = (char*)malloc(stat_buf.st_size);
    if (!file_data) {
        close(fd);
        return command_result_create(false, "Memory allocation failed!");
    }

    ssize_t bytes_read = read(fd, file_data, stat_buf.st_size);
    close(fd);

    if (bytes_read != stat_buf.st_size) {
        free(file_data);
        return command_result_create(false, "File read error");
    }

    char message[256];
    snprintf(message, sizeof(message), "File: %s, Size: %ld bytes", args, stat_buf.st_size);

    command_result_t* result = command_result_create_with_data(true, message, file_data, bytes_read);
    free(file_data);

    return result;
}

// 删除文件或目录
command_result_t* nd_remove(command_context_t* context, const char* args) {
    if (!context || !args) {
        return command_result_create(false, "Invalid arguments");
    }

    char full_path[4096];
    snprintf(full_path, sizeof(full_path), "%s/%s", context->current_dir, args);

    // 安全检查
    if (!is_path_safe(context, full_path)) {
        return command_result_create(false, "Path traversal not allowed");
    }

    // 使用系统调用 unlink 或 rmdir
    struct stat stat_buf;
    if (stat(full_path, &stat_buf) != 0) {
        return command_result_create(false, strerror(errno));
    }

    int result;
    if (S_ISDIR(stat_buf.st_mode)) {
        result = rmdir(full_path);
    } else {
        result = unlink(full_path);
    }

    if (result == 0) {
        char message[256];
        snprintf(message, sizeof(message), "Successfully remove: %s", args);
        return command_result_create(true, message);
    } else {
        return command_result_create(false, strerror(errno));
    }
}

// 创建目录
command_result_t* nd_mkdir(command_context_t* context, const char* args) {
    if (!context || !args) {
        return command_result_create(false, "Invalid arguments");
    }

    char full_path[4096];
    snprintf(full_path, sizeof(full_path), "%s/%s", context->current_dir, args);

    // 安全检查
    if (!is_path_safe(context, full_path)) {
        return command_result_create(false, "Path traversal not allowed");
    }

    // 使用系统调用mkdir    
    if (mkdir(full_path, 0775) == 0) {
        char message[256];
        snprintf(message, sizeof(message), "Directory created: %s", args);
        return command_result_create(true, message);
    } else {
        return command_result_create(false, strerror(errno));
    }
}
