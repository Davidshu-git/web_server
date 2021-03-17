/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_CONNECTOR_H
#define WEB_SERVER_NET_CONNECTOR_H

#include <memory>
#include <functional>

#include "base/Noncopyable.h"
#include "net/InetAddress.h"

namespace web_server {

namespace net {

class Channel;
class EventLoop;

class Connector : private Noncopyable,
                  public std::enable_shared_from_this<Connector> {
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;
    Connector(EventLoop *loop, const InetAddress &server_addr);
    ~Connector();

    void set_new_connection_callback(const NewConnectionCallback &cb) {
        new_connection_callback_ = cb;
    }
    // call in any thread
    void start();

    // call in loop thread
    void restart();

    // call in any thread
    void stop();
    const InetAddress &server_address() const {
        return server_addr_;
    }
private:
    enum States { kDisconnected, kConnecting, kConnected };
    static const int k_max_retry_delay_ms = 30 * 1000;
    static const int k_init_retry_delay_ms = 500;
    
    EventLoop *loop_;
    InetAddress server_addr_;
    bool connect_;
    States state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback new_connection_callback_;
    int retry_delay_ms_;

    void set_state(States s) {
        state_ = s;
    }

    void start_in_loop();
    void stop_in_loop();
    void connect();
    void connecting(int sockfd);
    void handle_write();
    void handle_error();
    void retry(int sockfd);
    int remove_and_reset_channel();
    void reset_channel();
};

} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_CONNECTOR_H