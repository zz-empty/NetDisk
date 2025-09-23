# NetDisk/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(NetDisk VERSION 1.0.0)

# 设置C标准
set(CMAKE_C_STANDARD 11)

# 设置构建类型（默认为Release）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加服务器和客户端子目录
add_subdirectory(server)
add_subdirectory(client)

# 显示配置信息
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/bin")

