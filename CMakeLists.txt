# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.16.3)

# 项目信息
project(web_server VERSION 1.0)

# 添加编译选项
add_definitions(-std=c++11)

# 指定项目搜索include文件目录
include_directories("${PROJECT_SOURCE_DIR}")

# 添加子目录
add_subdirectory(base)
add_subdirectory(net)



