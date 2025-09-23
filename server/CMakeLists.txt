# server/CMakeLists.txt

# 设置包含目录
include_directories(include)

# 查找源文件
file(GLOB SERVER_SOURCES "src/*.c")
file(GLOB TEST_SOURCES "test/*.c")

# 添加主服务器可执行文件
add_executable(NetDiskServer ${SERVER_SOURCES})

# 添加测试可执行文件（如果有测试文件）
if(TEST_SOURCES)
    add_executable(ConfigTest test/test_config.c src/config.c)
    target_compile_options(ConfigTest PRIVATE -Wall -Wextra -Werror)
endif()

# 设置可执行文件属性
set_target_properties(NetDiskServer PROPERTIES
    OUTPUT_NAME "NetDiskServer"
)

# 添加编译选项
target_compile_options(NetDiskServer PRIVATE 
    -Wall -Wextra -Werror
)

# 安装目标
install(TARGETS NetDiskServer DESTINATION bin)

