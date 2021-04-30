/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include <cstdio>
#include <string>
#include <unistd.h>

#include "base/Logging.h"
#include "base/CurrentThread.h"
#include "base/Thread.h"
#include "net/TcpConnection.h"
#include "net/EventLoop.h"
#include "net/TcpServer.h"

using namespace web_server;
using namespace web_server::net;

int num_threads = 0;

class EchoServer {
public:
    EchoServer(EventLoop *loop, const InetAddress &listen_addr) 
    : loop_(loop),
      server_(loop, listen_addr, "EchoServer") {
        server_.set_connection_callback(std::bind(&EchoServer::on_connection, this, _1));
        server_.set_message_callback(std::bind(&EchoServer::on_message, this, _1, _2, _3));
        server_.set_thread_num(num_threads);
    }

    void start() {
        server_.start();
    }

private:
    EventLoop *loop_;
    TcpServer server_;

    void on_connection(const TcpConnectionPtr &conn) {
        // LOG_TRACE << conn->peer_addr().to_IP_port() << " -> " << conn->local_addr().to_IP_port() << " is " << (conn->connected() ? "UP" : "DOWN");
        // LOG_INFO << conn->get_tcp_info_string();
        conn->send("hello\n");
    }

    void on_message(const TcpConnectionPtr &conn,
                    Buffer *buf, Timestamp time) {
        std::string msg(buf->retrieve_all_as_string());
        // LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << time.to_string();
        if (msg == "exit\n") {
            conn->send("bye\n");
            conn->shutdown();
        }
        if (msg == "quit\n") {
            loop_->quit();
        }
        conn->send(msg);
    }
};

int main(int argc, char *argv[]) {
    // LOG_INFO << "pid = " << getpid() << ", tid = " << current_thread::tid();
    // LOG_INFO << "sizeof TcpConnection = " << sizeof(TcpConnection);

    if (argc > 1) {
        num_threads = atoi(argv[1]);
    }
    EventLoop loop;
    InetAddress listen_addr(2000);
    EchoServer server(&loop, listen_addr);
    server.start();
    loop.loop();
}