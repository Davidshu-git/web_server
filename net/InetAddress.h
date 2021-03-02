/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_INETADDRESS_H
#define WEB_SERVER_NET_INETADDRESS_H

#include <netinet/in.h>
#include <cstdint>

#include <string>

#include "base/Copyable.h"

namespace web_server {

namespace net {

class InetAddress : public Copyable {
public:
    explicit InetAddress(uint16_t port = 0);
    InetAddress(const std::string &IP, uint16_t port);
    explicit InetAddress(const sockaddr_in &addr) : addr_(addr) {};

    std::string to_IP() const;
    std::string to_IP_port() const;
    const sockaddr_in& get_sock_addr() const {
        return addr_;
    };
    void set_sock_addr(const sockaddr_in &addr) {
        addr_ = addr;
    };

    uint32_t IP_net_Endian() const {
        return addr_.sin_addr.s_addr;
    };
    uint16_t port_net_Endian() const {
        return addr_.sin_port;
    };

private:
    struct sockaddr_in addr_;
};

} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_INETADDRESS_H