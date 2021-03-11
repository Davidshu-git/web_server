# 设定源文件变量
set(NET_SRCS
    EventLoop.cc
    Channel.cc
    Poller.cc
    poller/PollPoller.cc
    poller/DefaultPoller.cc
    poller/EPollPoller.cc
    InetAddress.cc
    Socket.cc
    Buffer.cc
    TcpConnection.cc
    Connector.cc
    Acceptor.cc
    Timer.cc
    TimerQueue.cc
    TcpServer.cc
    EventLoopThread.cc
)

# 生成net_lib库
add_library(net_lib ${NET_SRCS})

# 添加链接库
target_link_libraries(net_lib base_lib)

add_subdirectory(tests)

