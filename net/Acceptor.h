/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_ACCEPTOR_H
#define WEB_SERVER_NET_ACCEPTOR_H

#include <functional>

#include "base/Noncopyable.h"
#include "net/Socket.h"
#include "net/Channel.h"

namespace web_server {

namespace net {

class EventLoop;
class InetAddress;

/**
 * @brief 负责监听socketfd上的accept请求
 * 
 */
class Acceptor : private Noncopyable {
public:
    using NewConnectionCallback = std::function<void (int sockfd, const InetAddress &)>;
    Acceptor(EventLoop *loop, const InetAddress &listen_addr, bool reuse_port);
    ~Acceptor();

    void set_new_connection_callback(const NewConnectionCallback &cb) {
        new_connection_callback_ = cb;
    }

    void listen();
    bool is_listening() const {
        return listening_;
    }

private:
    EventLoop *loop_;
    Socket accept_socket_;
    Channel accept_channel_;
    NewConnectionCallback new_connection_callback_;
    bool listening_;
    int idle_fd_;

    void handle_read();
};

} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_ACCEPTOR_H