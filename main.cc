/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include <map>
#include <iostream>

#include "http/HttpServer.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "base/Logging.h"
#include "net/EventLoop.h"

using namespace web_server;
using namespace web_server::net;
using namespace web_server::http;

void on_request(const HttpRequest &req, HttpResponse *resp) {
    std::cout << "Headers " << req.method_string() << " " << req.path() << std::endl;
    const std::map<std::string, std::string> &headers = req.headers();
    for (const auto &header : headers) {
        std::cout << header.first << ": " << header.second << std::endl;
    }

    if (req.path() == "/") {
        resp->set_status_code(HttpResponse::k_200_ok);
        resp->set_status_message("OK");
        resp->set_content_type("text/html");
        resp->add_header("Server", "web_server");
        std::string now = Timestamp::now().to_formatted_string();
        resp->set_body("<html><head><title>This is title</title></head>"
                       "<body><h1>Hello</h1>Now is " + now +
                       "</body></html>");
    } else if (req.path() == "/hello") {
        resp->set_status_code(HttpResponse::k_200_ok);
        resp->set_status_message("OK");
        resp->set_content_type("text/plain");
        resp->set_body("hello, world!\n");
    } else {
        resp->set_status_code(HttpResponse::k_404_not_found);
        resp->set_status_message("Not Found");
        resp->set_close_connection(true);
    }
}

int main(int argc, char *argv[]) {
    int num_threads = 0;
    if (argc > 1) {
        Logger::set_log_level(Logger::ERROR);
        num_threads = atoi(argv[1]);
    }
    EventLoop loop;
    HttpServer server(&loop, InetAddress(8000), "http_server");
    server.set_http_callback(on_request);
    server.set_thread_num(num_threads);
    server.start();
    loop.loop();
}