/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/Acceptor.h"

#include <unistd.h>
#include <fcntl.h>
#include <cassert>

#include "base/Logging.h"
#include "net/Socket.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"

namespace web_server {

namespace net {

/**
 * @brief Construct a new Acceptor:: Acceptor object
 * 构建一个非阻塞的socket、设置其reuseaddr、bind步骤等
 * 通过得到的socket来创建一个channel对象
 * 设置其回调函数，handle_read是需要该类的用户自己进行编写的函数
 * @param loop 
 * @param listen_addr 
 * @param reuse_port 
 */
Acceptor::Acceptor(EventLoop *loop, const InetAddress &listen_addr, bool reuse_port)
    : loop_(loop),
      accept_socket_(sockets::create_nonblocking()),
      accept_channel_(loop, accept_socket_.fd()),
      listening_(false) {
    accept_socket_.set_reuse_addr(true);
    accept_socket_.bind_addr(listen_addr);
    accept_channel_.set_read_callback(std::bind(&Acceptor::handle_read, this));
}

Acceptor::~Acceptor() {
    accept_channel_.disable_all();
    accept_channel_.remove();
}

/**
 * @brief 负责创建TCP服务端步骤
 * 在IO线程中执行，设置socket为监听，用channel类设置其关注读事件
 */
void Acceptor::listen() {
    loop_->assert_in_loop_thread();
    listening_ = true;
    accept_socket_.listen();
    accept_channel_.enable_reading();
    // LOG_TRACE << "acceptor channel fd set";
}

/**
 * @brief 当检测到sockfd上的读事件时需要执行的回调函数
 * 需要在IO线程中执行，该类可由用户编写，但执行的行为其实都差不多
 * 使用此处的已经够用了，若new_connection_callback存在则调用这个函数
 * 若不存在，则相当于接收到了读就绪但什么都不做，此时关闭accept到的文件描述符
 */
void Acceptor:: handle_read() {
    loop_->assert_in_loop_thread();
    InetAddress peer_addr;
    int connd = accept_socket_.accept(&peer_addr);
    if (connd > 0) {
        if (new_connection_callback_) {
            new_connection_callback_(connd, peer_addr);
        } else {
            ::close(connd);
        }
    } else {
        // LOG_SYSERR << "in Acceptor::handle_read";
    }
}

} // namespace net

} // namespace web_server