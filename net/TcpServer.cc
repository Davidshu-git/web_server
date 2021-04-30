/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/TcpServer.h"

#include <cassert>

#include "base/Logging.h"
#include "net/EventLoop.h"
#include "net/Acceptor.h"
#include "net/EventLoopThreadPool.h"

namespace web_server {

namespace net {

/**
 * @brief Construct a new Tcp Server:: Tcp Server object
 * 使用acceptor获得连接，设置acceptor获得连接时执行的回调函数
 * @param loop 
 * @param listen_addr 
 * @param name 
 * @param option 
 */
TcpServer::TcpServer(EventLoop *loop, const InetAddress &listen_addr,
                     const std::string &name, Option option)
    : loop_(loop),
      IP_port_(listen_addr.to_IP_port()),
      name_(name),
      acceptor_(new Acceptor(loop, listen_addr, option == kReusePort)),
      thread_pool_(new EventLoopThreadPool(loop, name_)),
      connection_callback_(default_connection_callback),
      message_callback_(default_message_callback),
      next_conn_ID_(1) {
    acceptor_->set_new_connection_callback(std::bind(&TcpServer::new_connection, this, _1, _2));
}

TcpServer::~TcpServer() {
    loop_->assert_in_loop_thread();
    // LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

    for (auto &item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->get_loop()->run_in_loop(std::bind(&TcpConnection::connection_destroyed, conn));
    }
}

void TcpServer::set_thread_num(int num_threads) {
    assert(num_threads >= 0);
    thread_pool_->set_thread_num(num_threads);
}

void TcpServer::start() {
    if (started_.get_set(1) == 0) {
        thread_pool_->start(thread_init_callback_);
        assert(!acceptor_->is_listening());
        loop_->run_in_loop(std::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}

// 有新连接到来后的处理方式
void TcpServer::new_connection(int sockfd, const InetAddress &peer_addr) {
    loop_->assert_in_loop_thread();
    // 从线程池中取一个io线程
    EventLoop *IO_loop = thread_pool_->get_next_loop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", IP_port_.c_str(), next_conn_ID_);
    ++next_conn_ID_;
    std::string conn_name = name_ + buf;

    // LOG_INFO << "TcpServer::new_connection [" << name_ << "] - new connection [" << conn_name << "] from " << peer_addr.to_IP_port();

    InetAddress local_addr(InetAddress::get_local_addr(sockfd));
    // 根据已有的信息创建一个tcpconnection对象，该对象由线程池中的IO线程进行管理
    TcpConnectionPtr conn(new TcpConnection(IO_loop, conn_name, sockfd, local_addr, peer_addr));
    connections_[conn_name] = conn;
    conn->set_connection_callback(connection_callback_);
    conn->set_message_callback(message_callback_);
    conn->set_write_complete_callback(write_complete_callback_);
    conn->set_close_callback(std::bind(&TcpServer::remove_connection, this, _1));
    // 将连接建立后的任务交给线程池中的IO线程去完成
    IO_loop->run_in_loop(std::bind(&TcpConnection::connection_established, conn));
}

void TcpServer::remove_connection(const TcpConnectionPtr &conn) {
    loop_->run_in_loop(std::bind(&TcpServer::remove_connection_in_loop, this, conn));
}

void TcpServer::remove_connection_in_loop(const TcpConnectionPtr &conn) {
    loop_->assert_in_loop_thread();
    // LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_ << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    EventLoop *IO_loop = conn->get_loop();
    IO_loop->queue_in_loop(std::bind(&TcpConnection::connection_destroyed, conn));
}

} // namespace net

} // namespace web_server