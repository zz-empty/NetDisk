cmake_minimum_required(VERSION 3.10)
project(NetDisk C)

# 可选：指定 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 把所有生成的可执行文件放到 build/bin 目录，避免混乱
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 包含目录
include_directories(include)

# 查找源文件
file(GLOB SERVER_SOURCES "src/*.c")
file(GLOB TEST_SOURCES "test/*.c")

# 主服务器可执行文件
add_executable(NetDiskServer ${SERVER_SOURCES})
target_link_libraries(NetDiskServer PRIVATE pthread)
target_compile_options(NetDiskServer PRIVATE -pthread)

# 测试可执行文件（显式列出需要的源）
add_executable(NetworkTest test/test_network.c src/network.c src/config.c src/threadpool.c src/protocol.c src/command.c src/command_parser.c)
target_link_libraries(NetworkTest PRIVATE pthread)
target_compile_options(NetworkTest PRIVATE  -pthread)

add_executable(ConfigTest test/test_config.c src/config.c)
target_compile_options(ConfigTest PRIVATE)

add_executable(ThreadPoolTest test/test_threadpool.c src/threadpool.c)
target_link_libraries(ThreadPoolTest PRIVATE pthread)
target_compile_options(ThreadPoolTest PRIVATE -pthread)

add_executable(CommandTest test/test_commands.c src/command.c src/command_parser.c)

# 安装所有可执行文件到 bin
install(TARGETS NetDiskServer NetworkTest ConfigTest ThreadPoolTest CommandTest
        RUNTIME DESTINATION bin)

