# 设置当前目录源文件变量
set(BASE_SRCS
    Thread.cc
    CurrentThread.cc
)

# 生成base_lib库
add_library(base_lib ${BASE_SRCS})

# 指定链接库
target_link_libraries(base_lib pthread)

# 添加子目录
add_subdirectory(tests)