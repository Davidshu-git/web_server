/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/Acceptor.h"

#include "net/InetAddress.h"
#include "net/EventLoop.h"

using web_server::net::InetAddress;

void new_collection(int sockfd, const InetAddress &peer_addr) {
    printf("new_collection(): accepted a new connection from %s\n", peer_addr.to_IP_port().c_str());
    write(sockfd, "How are you?\n", 13);
    close(sockfd);
}

int main() {
    printf("main(): pid = %d\n", getpid());
    web_server::net::InetAddress listen_addr(9981);
    web_server::net::EventLoop loop;
    web_server::net::Acceptor acceptor(&loop, listen_addr, false);
    acceptor.set_new_connection_callback(new_collection);
    acceptor.listen();

    loop.loop();
}