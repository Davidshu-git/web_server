/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/TcpClient.h"

#include <cstdio>

#include "base/Logging.h"
#include "net/Connector.h"
#include "net/EventLoop.h"
#include "net/Socket.h"

namespace web_server {

namespace net {

namespace detail {

void remove_connection(EventLoop *loop, const TcpConnectionPtr &conn) {
    loop->queue_in_loop(std::bind(&TcpConnection::connection_destroyed, conn));
}

} // namespace detail

TcpClient::TcpClient(EventLoop *loop, const InetAddress &server_addr,
              const std::string &name)
    : loop_(loop),
      connector_(new Connector(loop, server_addr)),
      name_(name),
      connection_callback_(default_connection_callback),
      message_callback_(default_message_callback),
      retry_(false),
      connect_(true),
      next_conn_ID_(1) {
    connector_->set_new_connection_callback(std::bind(&TcpClient::new_connection, this, _1));
    // LOG_INFO << "TcpClient::TcpClient[" << name_ << "] - connector " << get_pointer(connector_);
}

TcpClient::~TcpClient() {
    // LOG_INFO << "TcpClient::~TcpClient[" << name_ << "] - connector " << get_pointer(connector_);
    TcpConnectionPtr conn;
    {
        MutexLockGuard lock(mutex_);
        conn = connection_;
    }

    if (conn) {
        assert(loop_ == conn->get_loop());
        CloseCallback cb = std::bind(&detail::remove_connection, loop_, _1);
        loop_->run_in_loop(std::bind(&TcpConnection::set_close_callback, conn, cb));
    } else {
        connector_->stop();
    }
}

void TcpClient::connect() {
    // LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to " << connector_->server_address().to_IP_port();
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;
    {
        MutexLockGuard lock(mutex_);
        if (connection_) {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

void TcpClient::new_connection(int sockfd) {
    loop_->assert_in_loop_thread();
    InetAddress peer_addr(InetAddress::get_peer_addr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peer_addr.to_IP_port().c_str(), next_conn_ID_);
    ++next_conn_ID_;
    std::string conn_name = name_ + buf;

    InetAddress local_addr(InetAddress::get_local_addr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(loop_, conn_name, sockfd, local_addr, peer_addr));
    conn->set_connection_callback(connection_callback_);
    conn->set_message_callback(message_callback_);
    conn->set_write_complete_callback(write_complete_callback_);
    conn->set_close_callback(std::bind(&TcpClient::remove_connection, this, _1));
    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }
    conn->connection_established();

}

void TcpClient::remove_connection(const TcpConnectionPtr &conn) {
    loop_->assert_in_loop_thread();
    assert(loop_ == conn->get_loop());
    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queue_in_loop(std::bind(&TcpConnection::connection_destroyed, conn));
    if (retry_ && connect_) {
        // LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to " << connector_->server_address().to_IP_port();
        connector_->restart();
    }
}

} // namespace net

} // namespace web_server