# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.16.3)

# 项目信息
project(web_server VERSION 1.0)

# 设定c++标准
set(CMAKE_CXX_STANDARD 17)

# 设定默认build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "debug")
endif()


# 设定编译选项
set(CXX_FLAGS
    -g
    -Wall
    -std=c++11
)

# 输出编译信息
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

# 添加子目录
add_subdirectory(base)
add_subdirectory(net)



