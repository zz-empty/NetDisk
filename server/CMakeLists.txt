# server/CMakeLists.txt

# 设置包含目录
include_directories(include)

# 查找源文件
file(GLOB SERVER_SOURCES "src/*.c")

# 检查是否有源文件
if(NOT SERVER_SOURCES)
    message(WARNING "No source files found in server/src/")
    # 创建一个空的源文件列表，至少包含main.c
    set(SERVER_SOURCES src/main.c)
endif()

# 添加可执行文件
add_executable(NetDiskServer ${SERVER_SOURCES})

# 设置可执行文件属性
set_target_properties(NetDiskServer PROPERTIES
    OUTPUT_NAME "NetDiskServer"
)

# 添加编译选项
target_compile_options(NetDiskServer PRIVATE 
    -Wall -Wextra -Werror
)

message(STATUS "Server sources: ${SERVER_SOURCES}")

