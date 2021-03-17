/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/Connector.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpClient.h"

using namespace web_server;
using namespace web_server::net;

EventLoop *g_loop;

void connect_callback(int sockfd) {
    printf("connected.\n");
    g_loop->quit();
}

int main(int argc, char *argv[]) {
    EventLoop loop;
    g_loop = &loop;
    InetAddress addr("127.0.0.1", 9981);
    ConnectorPtr connector(new Connector(&loop, addr));
    connector->set_new_connection_callback(connect_callback);
    connector->start();
    loop.loop();
}