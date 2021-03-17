/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_HTTP_HTTPSERVER_H
#define WEB_SERVER_HTTP_HTTPSERVER_H

#include <functional>

#include "base/Noncopyable.h"
#include "net/TcpServer.h"

namespace web_server {

namespace http {

using namespace web_server::net;
class HttpRequest;
class HttpResponse;

class HttpServer : private Noncopyable {
public:
    using HttpCallback = std::function<void(const HttpRequest &, HttpResponse *)>;
    HttpServer(EventLoop *loop,
               const InetAddress &listen_addr,
               const std::string &name,
               TcpServer::Option option = TcpServer::kNoReusePort);
    
    EventLoop *getloop() const {
        return server_.get_loop();
    }

    void set_http_callback(const HttpCallback &cb) {
        http_callback_ = cb;
    }

    void set_thread_num(int num_threads) {
        server_.set_thread_num(num_threads);
    }

    void start();

private:
    TcpServer server_;
    HttpCallback http_callback_;

    void on_connetion(const TcpConnectionPtr &conn);
    void on_message(const TcpConnectionPtr &conn,
                    Buffer *buf,
                    Timestamp receive_time);
    void on_request(const TcpConnectionPtr &, const HttpRequest &);
};

} // namespace http

} // namespace web_server

#endif // WEB_SERVER_HTTP_HTTPSERVER_H