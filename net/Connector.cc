/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/Connector.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <functional>
#include <cerrno>

#include "base/Logging.h"
#include "net/EventLoop.h"
#include "net/Channel.h"
#include "net/Socket.h"

namespace web_server {

namespace net {

Connector::Connector(EventLoop *loop, const InetAddress &server_addr)
    : loop_(loop),
      server_addr_(server_addr),
      connect_(false),
      state_(kDisconnected),
      retry_delay_ms_(k_init_retry_delay_ms) {
    LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector() {
    LOG_DEBUG << "dtor[" << this << "]";
    assert(!channel_);
}

void Connector::start() {
    connect_ = true;
    loop_->run_in_loop(std::bind(&Connector::start_in_loop, this));
}

void Connector::restart() {
    loop_->assert_in_loop_thread();
    set_state(kDisconnected);
    retry_delay_ms_ = k_init_retry_delay_ms;
    connect_ = true;
    start_in_loop();
}

void Connector::stop() {
    connect_ = false;
    loop_->queue_in_loop(std::bind(&Connector::stop_in_loop, this));
}

void Connector::start_in_loop() {
    loop_->assert_in_loop_thread();
    assert(state_ == kDisconnected);
    if (connect_) {
        connect();
    } else {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::stop_in_loop() {
    loop_->assert_in_loop_thread();
    if (state_ == kConnecting) {
        set_state(kDisconnected);
        int sockfd = remove_and_reset_channel();
        retry(sockfd);
    }
}

void Connector::connect() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_SYSFATAL << "Connect::connect";
    }
    sockaddr_in addr = server_addr_.get_sock_addr();
    int ret = ::connect(sockfd,
                           reinterpret_cast<sockaddr *>(&addr),
                           static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    int saved_errno = (ret == 0) ? 0 : errno;
    switch (saved_errno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;
    
    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;
    
    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        LOG_SYSERR << "connect error in Connector::startInLoop " << saved_errno;
        ::close(sockfd);
        break;

    default:
        LOG_SYSERR << "Unexpected error in Connector::startInLoop " << saved_errno;
        ::close(sockfd);
        break;
    }
}

void Connector::connecting(int sockfd) {
    set_state(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->set_write_callback(std::bind(&Connector::handle_write, this));
    channel_->set_error_callback(std::bind(&Connector::handle_error, this));
    channel_->enable_writing();
}

void Connector::handle_write() {
    LOG_TRACE << "Connector::handle_write " << state_;
    if (state_ == kConnecting) {
        int sockfd = remove_and_reset_channel();
        int err = sockets::get_socket_error(sockfd);
        if (err) {
            LOG_WARN << "Connector::handle_write - SO_ERROR = "
                     << err << " " << strerror_tl(err);
            retry(sockfd);
        } else if (sockets::is_self_connect(sockfd)) {
            LOG_WARN << "Connector::handle_write - Self connect";
            retry(sockfd);
        } else {
            set_state(kConnected);
            if (connect_) {
                new_connection_callback_(sockfd);
            } else {
                ::close(sockfd);
            }
        }
    } else {
        assert(state_ == kDisconnected);
    }
}

void Connector::handle_error() {
    LOG_ERROR << "Connector::handle_error state=" << state_;
    if (state_ == kConnecting) {
        int sockfd = remove_and_reset_channel();
        int err = sockets::get_socket_error(sockfd);
        LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd) {
    ::close(sockfd);
    set_state(kDisconnected);
    if (connect_) {
        LOG_INFO << "Connector::retry - Retry connecting to " << server_addr_.to_IP_port()
                 << " in " << retry_delay_ms_ << " milliseconds. ";
        loop_->run_after(retry_delay_ms_ / 1000.0,
                         std::bind(&Connector::start_in_loop, shared_from_this()));
        retry_delay_ms_ = std::min(retry_delay_ms_ * 2, k_max_retry_delay_ms);
    } else {
        LOG_DEBUG << "do not connect";
    }
}

int Connector::remove_and_reset_channel() {
    channel_->disable_all();
    channel_->remove();
    int sockfd = channel_->fd();
    loop_->queue_in_loop(std::bind(&Connector::reset_channel, this));
    return sockfd;
}

void Connector::reset_channel() {
    channel_.reset();
}

} // namespace net

} // namespace web_server