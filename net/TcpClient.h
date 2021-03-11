/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_TCPCLIENT_H
#define WEB_SERVER_NET_TCPCLIENT_H

#include "base/Mutex.h"
#include "net/TcpConnection.h"

namespace web_server {

namespace net {

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient : private Noncopyable {
public:
    TcpClient(EventLoop *loop, const InetAddress &server_addr,
              const std::string &name);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const {
        MutexLockGuard lock(mutex_);
        return connection_;
    }

    EventLoop *get_loop() const {
        return loop_;
    }

    bool retry() const {
        return retry_;
    }

    void enable_retry() {
        retry_ = true;
    }

    void set_connection_callback(ConnectionCallback cb) {
        connection_callback_ = cb;
    }

    void set_message_callback(MessageCallback cb) {
        message_callback_ = cb;
    }

    void set_write_complete_callback(WriteCompleteCallback cb) {
        write_complete_callback_ = cb;
    }

private:
    EventLoop *loop_;
    ConnectorPtr connector_;
    const std::string name_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    bool retry_;
    bool connect_;
    int next_conn_ID_;
    mutable MutexLock mutex_;
    TcpConnectionPtr connection_;

    void new_connection(int sockfd);
    void remove_connection(const TcpConnectionPtr &conn);
};

} // namespace net

} // namespace web_server

#endif // WEB_SERVER_NET_TCPCLIENT_H