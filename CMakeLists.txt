# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.16)

# 项目信息
project(web_server)

set(CMAKE_CXX_STANDARD 17)

# 指定生成目标
add_executable(web_server main.cc)

# 添加子目录
add_subdirectory(base)
add_subdirectory(net)