/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/InetAddress.h"

#include <string>
#include <arpa/inet.h>

#include "base/Logging.h"

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

namespace web_server {

namespace net {

InetAddress::InetAddress(uint16_t port) {
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = INADDR_ANY;
    addr_.sin_port = htobe16(port);
}

InetAddress::InetAddress(const std::string &IP, uint16_t port) {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htobe16(port);
    if (::inet_pton(AF_INET, IP.c_str(), &addr_.sin_addr) <= 0) {
        LOG_SYSERR << "InetAddress::InetAddress(const std::string &IP, uint16_t port)";
    }
}

std::string InetAddress::to_IP() const {
    const int size = 32;
    char buf[size] = "";
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(size));
    return buf;
}

std::string InetAddress::to_IP_port() const {
    const int size = 32;
    char buf[size] = "";

    char host[16] = "INVALID";
    ::inet_ntop(AF_INET, &addr_.sin_addr, host, static_cast<socklen_t>(16));

    uint16_t port = be16toh(addr_.sin_port);
    snprintf(buf, size, "%s:%u", host, port);
    return buf;
}

} // namespace net

} // namespace web_server