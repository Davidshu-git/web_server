/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_TCPCONNECTION_H
#define WEB_SERVER_NET_TCPCONNECTION_H

#include <memory>
#include <string>

#include "base/Noncopyable.h"
#include "base/Logging.h"
#include "net/Buffer.h"
#include "net/InetAddress.h"

namespace web_server {

namespace net {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : private Noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    
private:
    enum StateE {kDisconnected, kConnecting, kConnected, kDisconnecting};
    void handle_read(Timestamp receive_time);
    void handle_write();
    void handle_close();
    void handle_error();
    
    void send_in_loop(const std::string &message);
    void shut_down_in_loop();

    void set_state(StateE s) {

    }
    
    const std::string state_to_string() const;

    EventLoop *loop_;
    const std::string name_;
    StateE state_;
    bool reading_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress local_addr_;
    const InetAddress peer_addr_;


};

} // namespace net

} // namespace web_server

#endif // WEB_SERVER_NET_TCPCONNECTION_H