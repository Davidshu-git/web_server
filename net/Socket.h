/**
 * @brief socket head file
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_SOCKET_H
#define WEB_SERVER_NET_SOCKET_H

#include "base/Noncopyable.h"

struct tcp_info;

namespace web_server {

namespace net {

class InetAddress;

class Socket : private Noncopyable {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();

    int fd() const {
        return sockfd_;
    }
    bool get_tcp_info(struct tcp_info *) const;
    bool get_tcp_info_string(char *buf, int len) const;

    void bind_addr(const InetAddress &local_addr);
    void listen();
    int accept(InetAddress *peer_addr);

    void shutdown_write();
    void set_tcp_no_delay(bool on);
    void set_reuse_addr(bool on);
    void set_keep_alive(bool on);
private:
    const int sockfd_;
};

} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_SOCKET_H