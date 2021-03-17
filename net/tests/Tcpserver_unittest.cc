/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include <net/TcpServer.h>
#include <net/EventLoop.h>
#include <net/Buffer.h>
#include <base/Timestamp.h>

using web_server::net::InetAddress;
using web_server::net::EventLoop;
using web_server::net::TcpServer;
using web_server::net::TcpConnectionPtr;
using web_server::net::Buffer;
using web_server::Timestamp;

void on_connection(const TcpConnectionPtr &conn) {
    if (conn->connected()) {
        printf("on_connection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peer_addr().to_IP_port().c_str());
    } else {
        printf("on_connection(): connetion [%s] is down\n", conn->name().c_str());
    }
}

void on_message(const TcpConnectionPtr &conn,
                Buffer *buf,
                Timestamp time) {
    printf("on_message(): received %zd bytes from connection [%s] at %s\n",
           buf->readable_bytes(), conn->name().c_str(), time.to_formatted_string().c_str());
}

int main() {
    printf("main(): pid = %d\n", getpid());
    InetAddress listen_addr(9981);
    EventLoop loop;

    TcpServer server(&loop, listen_addr, "tcp_server");
    server.set_connection_callback(on_connection);
    server.set_message_callback(on_message);
    server.start();

    loop.loop();
}