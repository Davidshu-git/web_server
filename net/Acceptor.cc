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

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listen_addr, bool reuse_port)
    : loop_(loop),
      accept_socket_(sockets::create_nonblocking()),
      accept_channel_(loop, accept_socket_.fd()),
      listening_(false) {
    accept_socket_.set_reuse_addr(true);
    //set_reuseport;
    accept_socket_.bind_addr(listen_addr);
    accept_channel_.set_read_callback(std::bind(&Acceptor::handle_read, this));
}

Acceptor::~Acceptor() {
    accept_channel_.disable_all();
    accept_channel_.remove();
}

/**
 * @brief 负责创建TCP服务端步骤
 * 在IO线程中执行
 */
void Acceptor::listen() {
    loop_->assert_in_loop_thread();
    listening_ = true;
    accept_socket_.listen();
    accept_channel_.enable_reading();
}

/**
 * @brief 当检测到sockfd上的读事件时需要执行的回调函数
 * 需要在IO线程中执行
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
        LOG_SYSERR << "in Acceptor::handle_read";
    }
}

} // namespace net

} // namespace web_server