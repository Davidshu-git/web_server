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
      listening_(false),
      idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(idle_fd_ >= 0);
    accept_socket_.set_reuse_addr(true);
    //set_reuseport;
    accept_socket_.bind_addr(listen_addr);
    accept_channel_.set_read_callback(std::bind(&Acceptor::handle_read, this));
}

Acceptor::~Acceptor() {
    accept_channel_.disable_all();
    accept_channel_.remove();
    ::close(idle_fd_);
}

void Acceptor::listen() {
    loop_->assert_in_loop_thread();
    listening_ = true;
    accept_socket_.listen();
    accept_channel_.enable_reading();
}

void Acceptor:: handle_read() {
    loop_->assert_in_loop_thread();
    InetAddress peer_addr;
    int connd = accept_socket_.accept(& peer_addr);
    if (connd > 0) {
        if (new_connection_callback_) {
            new_connection_callback_(connd, peer_addr);
        } else {
            ::close(connd);
        }
    } else {
        LOG_SYSERR << "in Acceptor::handle_read";
        if (errno == EMFILE) {
            ::close(idle_fd_);
            idle_fd_ = ::accept(accept_socket_.fd(), NULL, NULL);
            ::close(idle_fd_);
            idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

} // namespace net

} // namespace web_server