# 设定源文件变量
set(NET_SRCS
    EventLoop.cc
)

# 生成net_lib库
add_library(net_lib ${NET_SRCS})

# 添加链接库
target_link_libraries(net_lib base_lib)