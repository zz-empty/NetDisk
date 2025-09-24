# server/CMakeLists.txt

# 设置包含目录
include_directories(include)

# 查找源文件
file(GLOB SERVER_SOURCES "src/*.c")
file(GLOB TEST_SOURCES "test/*.c")

# 添加主服务器可执行文件
add_executable(NetDiskServer src/main.c src/config.c)
target_link_libraries(NetDiskServer pthread)  # 添加链接库

# 添加线程池测试可执行文件
add_executable(ThreadPoolTest test/test_threadpool.c src/threadpool.c)
target_link_libraries(ThreadPoolTest pthread)

# 分别设置可执行文件属性
set_target_properties(NetDiskServer PROPERTIES OUTPUT_NAME "NetDiskServer")
set_target_properties(ThreadPoolTest PROPERTIES OUTPUT_NAME "ThreadPoolTest")

# 添加编译选项
target_compile_options(NetDiskServer PRIVATE -Wall -Wextra -Werror)
target_compile_options(ThreadPoolTest PRIVATE -Wall -Wextra -Werror)

# 安装目标
install(TARGETS NetDiskServer DESTINATION bin)
