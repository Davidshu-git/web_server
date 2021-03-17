/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "http/HttpServer.h"

#include "net/EventLoop.h"
#include "base/Logging.h"
#include "http/HttpRequest.h"
#include "http/HttpContext.h"
#include "http/HttpResponse.h"

namespace web_server {

namespace http {

namespace detail {

void default_http_callback(const HttpRequest &, HttpResponse *resp) {
    resp->set_status_code(HttpResponse::k_404_not_found);
    resp->set_status_message("Not Found");
    resp->set_close_connection(true);
}

} // namespace detail

HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listen_addr,
                       const std::string &name,
                       TcpServer::Option option)
    : server_(loop, listen_addr, name, option),
      http_callback_(detail::default_http_callback) {
    server_.set_connection_callback(
        std::bind(&HttpServer::on_connetion, this, _1));
    server_.set_message_callback(
        std::bind(&HttpServer::on_message, this, _1, _2, _3));
}

void HttpServer::start() {
    LOG_WARN << "HttpServer[" << server_.name()
             << "] starts listening on " << server_.IP_port();
    server_.start();
}

void HttpServer::on_connetion(const TcpConnectionPtr &conn) {
    if (conn->connected()) {
        conn->set_context(HttpContext());
    }
}

void HttpServer::on_message(const TcpConnectionPtr &conn,
                            Buffer *buf,
                            Timestamp receive_time) {
    HttpContext *context = boost::any_cast<HttpContext>(conn->get_mutable_context());

    if (!context->parse_request(buf, receive_time)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context->got_all()) {
        on_request(conn, context->request());
        context->reset();
    }
}

void HttpServer::on_request(const TcpConnectionPtr & conn,
                            const HttpRequest &req) {
    const std::string &connection = req.get_header("Connection");
    bool close = connection == "close" ||
        (req.get_version() == HttpRequest::k_http10 && connection != "Keep-Alive");
    HttpResponse response(close);
    http_callback_(req, &response);
    Buffer buf;
    response.append_to_buffer(&buf);
    conn->send(buf.to_string());

    if (response.close_connection()) {
        conn->shutdown();
    }
}

} // namespace http

} // namespace web_server