/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_TCPSERVER_H
#define WEB_SERVER_NET_TCPSERVER_H

#include <map>
#include <string>
#include <memory>
#include <functional>
#include <cassert>

#include "base/Noncopyable.h"
#include "base/Atomic.h"
#include "net/Callbacks.h"
#include "net/InetAddress.h"
#include "net/TcpConnection.h"

namespace web_server {

namespace net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

/**
 * @brief 管理tcpconnection对象
 * 
 */
class TcpServer : private Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    enum Option { kNoReusePort, kReusePort };

    TcpServer(EventLoop *loop, const InetAddress &listen_addr,
              const std::string &name, Option option = kNoReusePort);
    ~TcpServer();

    const std::string &IP_port() const {
        return IP_port_;
    }

    const std::string &name() const {
        return name_;
    }

    EventLoop *get_loop() const {
        return loop_;
    }

    void set_thread_num(int num_threads);
    void set_thread_init_callback(const ThreadInitCallback &cb) {
        thread_init_callback_ = cb;
    }

    std::shared_ptr<EventLoopThreadPool> thread_pool() {
        return thread_pool_;
    }
    
    void start();

    void set_connection_callback(const ConnectionCallback &cb) {
        connection_callback_ = cb;
    }

    void set_message_callback(const MessageCallback &cb) {
        message_callback_ = cb;
    }

    void set_write_complete_callback(const WriteCompleteCallback &cb) {
        write_complete_callback_ = cb;
    }
    
private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop *loop_;
    const std::string IP_port_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> thread_pool_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    ThreadInitCallback thread_init_callback_;
    AtomicInt32 started_;
    int next_conn_ID_;
    ConnectionMap connections_;

    void new_connection(int sockfd, const InetAddress &peer_addr);
    void remove_connection(const TcpConnectionPtr &conn);
    void remove_connection_in_loop(const TcpConnectionPtr &conn);
};

} // namespace net

} // namespace web_server

#endif // WEB_SERVER_NET_TCPSERVER_H