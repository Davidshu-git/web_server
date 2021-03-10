/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/Socket.h"

#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <cstdio>

#include "base/Logging.h"
#include "net/InetAddress.h"

namespace web_server {

namespace net {

Socket::~Socket() {
    if (::close(sockfd_) < 0) {
        LOG_SYSERR << "Socket::~Socket()";
    }
}

bool Socket::get_tcp_info(struct tcp_info *tcpi) const {
    socklen_t len = sizeof(*tcpi);
    memset(tcpi, 0, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool Socket::get_tcp_info_string(char *buf, int len) const {
    struct tcp_info tcpi;
    bool ok = get_tcp_info(&tcpi);
    if (ok) {
        snprintf(buf, len, "unrecovered=%u "
                 "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                 "lost=%u retrans=%u rtt=%u rttvar=%u "
                 "sshthresh=%u cwnd=%u total_retrans=%u",
                 tcpi.tcpi_retransmits,
                 tcpi.tcpi_rto,
                 tcpi.tcpi_ato,
                 tcpi.tcpi_snd_mss,
                 tcpi.tcpi_rcv_mss,
                 tcpi.tcpi_lost,         // Lost packets
                 tcpi.tcpi_retrans,      // Retransmitted packets out
                 tcpi.tcpi_rtt,          // Smoothed round trip time in usec
                 tcpi.tcpi_rttvar,       // Medium deviation
                 tcpi.tcpi_snd_ssthresh,
                 tcpi.tcpi_snd_cwnd,
                 tcpi.tcpi_total_retrans);
    }
    return ok;
}

void Socket::bind_addr(const InetAddress &local_addr) {
    sockaddr_in addr = local_addr.get_sock_addr();
    int ret = ::bind(sockfd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (ret < 0) {
        LOG_SYSFATAL << "Socket::bind_addr";
    }
}

void Socket::listen() {
    int ret = ::listen(sockfd_, SOMAXCONN);
    if (ret < 0) {
        LOG_SYSFATAL << "Socket::listen";
    }
}

int Socket::accept(InetAddress *peeraddr) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addr_len = static_cast<socklen_t>(sizeof(addr));
    int connfd = ::accept4(sockfd_, reinterpret_cast<sockaddr*>(&addr),
                           &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0) {
        peeraddr->set_sock_addr(addr);
        return connfd;
    } else {
        int saved_errno = errno;
        LOG_SYSERR << "Socket::accept";
        switch (saved_errno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                errno = saved_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                LOG_FATAL << "unexpected error of ::accept " << saved_errno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept " << saved_errno;
                break;
        }
    }
    return -1;
}

void Socket::shutdown_write() {
    if (shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_SYSERR << "Socket::shutdown_write";
    }
}

void Socket::set_tcp_no_delay(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &opt,
                 static_cast<socklen_t>(sizeof opt));
}

void Socket::set_reuse_addr(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt,
                 static_cast<socklen_t>(sizeof opt));
}

void Socket::set_keep_alive(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &opt,
                 static_cast<socklen_t>(sizeof opt));
}

namespace sockets {

int get_socket_error(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

bool is_self_connect(int sockfd) {
    struct sockaddr_in local_addr = InetAddress::get_local_addr(sockfd);
    struct sockaddr_in peer_addr = InetAddress::get_peer_addr(sockfd);
    return local_addr.sin_port == peer_addr.sin_port &&
           local_addr.sin_addr.s_addr == peer_addr.sin_addr.s_addr;
}

int create_nonblocking() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_SYSFATAL << "sockets::create_nonblocking";
    }
    return sockfd;
}

} // namespace sockets

} // namespace net

} // namespace web_sever