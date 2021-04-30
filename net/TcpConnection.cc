/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/TcpConnection.h"

#include <cassert>
#include <cerrno>

#include "base/Logging.h"
#include "net/Socket.h"
#include "net/Channel.h"
#include "net/EventLoop.h"

namespace web_server {

namespace net {

void default_connection_callback(const TcpConnectionPtr &conn) {
    // LOG_TRACE << conn->local_addr().to_IP_port() << " -> " << conn->peer_addr().to_IP_port() << " is " << (conn->connected() ? "UP" : "DOWN");
}

void default_message_callback(const TcpConnectionPtr &,
                              Buffer *buf,
                              Timestamp) {
    buf->retrieve_all();
}

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &name,
                             int sockfd,
                             const InetAddress &local_addr,
                             const InetAddress &peer_addr) 
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      local_addr_(local_addr),
      peer_addr_(peer_addr),
      high_water_mark_(64 * 1024 * 1024) {
    channel_->set_read_callback(std::bind(&TcpConnection::handle_read, this, _1));
    channel_->set_write_callback(std::bind(&TcpConnection::handle_write, this));
    channel_->set_close_callback(std::bind(&TcpConnection::handle_close, this));
    channel_->set_error_callback(std::bind(&TcpConnection::handle_error, this));
    // LOG_DEBUG << "TcpConnection::ctor[" <<  name_ << "] at " << this << " fd=" << sockfd;
    socket_->set_keep_alive(true);
}

TcpConnection::~TcpConnection() {
    // LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this << " fd=" << channel_->fd() << " state=" << state_to_string();
    assert(state_ == kDisconnected);
}

std::string TcpConnection::get_tcp_info_string() const {
    char buf[1024];
    buf[0] = '\0';
    socket_->get_tcp_info_string(buf, sizeof buf);
    return buf;
}

void TcpConnection::send(const std::string &message) {
    if (state_ == kConnected) {
        if (loop_->is_in_loop_thread()) {
            send_in_loop(message);
        } else {
            loop_->run_in_loop(std::bind(&TcpConnection::send_in_loop, this, message));
        }
    }
}

void TcpConnection::send(const void *message, size_t len) {
    const char *cStr = static_cast<const char*>(message);
    std::string str(cStr, len);
    send(str);
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        set_state(kDisconnecting);
        loop_->run_in_loop(std::bind(&TcpConnection::shutdown_in_loop, shared_from_this()));
    }
}

void TcpConnection::set_tcp_no_delay(bool on) {
    socket_->set_tcp_no_delay(on);
}

void TcpConnection::connection_established() {
    loop_->assert_in_loop_thread();
    assert(state_ == kConnecting);
    set_state(kConnected);
    channel_->tie(shared_from_this());
    channel_->enable_reading();
    connection_callback_(shared_from_this());
}

void TcpConnection::connection_destroyed() {
    loop_->assert_in_loop_thread();
    if (state_ == kConnected) {
        set_state(kDisconnected);
        channel_->disable_all();
        connection_callback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handle_read(Timestamp receive_time) {
    loop_->assert_in_loop_thread();
    int saved_errno = 0;
    ssize_t n = input_buffer_.read_fd(channel_->fd(), &saved_errno);
    if (n > 0) {
        message_callback_(shared_from_this(), &input_buffer_, receive_time);
    } else if (n == 0) {
        handle_close();
    } else {
        errno = saved_errno;
        // LOG_SYSERR << "TcpConnection::handle_read";
        handle_error();
    }
}

void TcpConnection::handle_write() {
    loop_->assert_in_loop_thread();
    if (channel_->is_writing()) {
        ssize_t n = ::write(channel_->fd(),
                            output_buffer_.peek(),
                            output_buffer_.readable_bytes());
        if (n > 0) {
            output_buffer_.retrieve(static_cast<size_t>(n));
            if (output_buffer_.readable_bytes() == 0) {
                channel_->disable_writing();
                if (write_complete_callback_) {
                    loop_->queue_in_loop(std::bind(write_complete_callback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdown_in_loop();
                }
            }
        } else {
            // LOG_SYSERR << "TcpConnection::handle_write";
        }
    } else {
        // LOG_TRACE << "Connection fd = " << channel_->fd() << " is down, no more writing";
    }
}

void TcpConnection::handle_close() {
    loop_->assert_in_loop_thread();
    // LOG_TRACE << "fd = " << channel_->fd() << " state = " << state_to_string();
    assert(state_ == kConnected || state_ == kDisconnecting);
    set_state(kDisconnected);
    channel_->disable_all();
    // 使用shared_ptr来管理this指针，避免this指向的对象生命周期提前结束
    close_callback_(shared_from_this());
}

void TcpConnection::handle_error(){
    int err = sockets::get_socket_error(channel_->fd());
    // LOG_ERROR << "TcpConnection::handle_error [" << name_ << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::send_in_loop(const std::string &message){
    loop_->assert_in_loop_thread();
    ssize_t n = 0;
    size_t remain = message.size();
    if (state_ == kDisconnected) {
        // LOG_WARN << "disconnected, give up writing";
        return;
    }
    // 若channel没有关注写事件，输出缓冲区没有数据可读，尝试直接对该文件描述符进行写操作
    if (!channel_->is_writing() && output_buffer_.readable_bytes() == 0) {
        n = ::write(channel_->fd(), message.data(), message.size());
        if (n >= 0) {
            remain = message.size() - n;
            if (remain == 0 && write_complete_callback_) {
                loop_->queue_in_loop(std::bind(write_complete_callback_, shared_from_this()));
            }
        } else {
            n = 0;
            if (errno != EWOULDBLOCK) {
                // LOG_SYSERR << "TcpConnection::send_in_loop";
            }
        }
    }

    assert(remain <= message.size());
    // 若一次性没有写完，则将剩余数据放到输出buffer中，然后让channel监听写事件，负责将剩余数据写出
    // 在handle_write中完成剩余工作
    if (remain > 0) {
        size_t old_len = output_buffer_.readable_bytes();
        if (old_len + remain >= high_water_mark_
            && old_len < high_water_mark_
            && high_water_mark_callback_) {
            loop_->queue_in_loop(std::bind(high_water_mark_callback_, shared_from_this(), old_len + remain));
        }
        output_buffer_.append(message.data() + n, remain);
        if (!channel_->is_writing()) {
            channel_->enable_writing();
        }
    }
    
}

void TcpConnection::shutdown_in_loop() {
    loop_->assert_in_loop_thread();
    if (!channel_->is_writing()) {
        socket_->shutdown_write();
    }
}

const std::string TcpConnection::state_to_string() const {
    switch (state_) {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

} // namespace net

} // namespace web_server