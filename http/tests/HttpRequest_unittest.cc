/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include <string>
#include <cassert>

#include "http/HttpContext.h"
#include "net/Buffer.h"

using std::string;
using web_server::Timestamp;
using web_server::net::Buffer;
using web_server::http::HttpContext;
using web_server::http::HttpRequest;

int main() {
    printf("start httprequest test\n");
    // test one
    HttpContext context;
    Buffer input;
    input.append("GET / HTTP/1.1\r\n"
                 "Host: code-david.cn\r\n"
                 "\r\n");
    assert(context.parse_request(&input, Timestamp::now()));
    assert(context.got_all());
    const HttpRequest &request = context.request();
    assert(request.method() == HttpRequest::k_get);
    assert(request.path() == string("/"));
    assert(request.get_version() == HttpRequest::k_http11);
    assert(request.get_header("Host") == string("code-david.cn"));
    assert(request.get_header("User-Agent") == string(""));

    // test int two pieces
    {
        string all("GET / HTTP/1.1\r\n"
                   "Host: code-david.cn\r\n"
                   "\r\n");
        for (size_t size1 = 0; size1 < all.size(); ++size1) {
            HttpContext context;
            Buffer input;
            input.append(all.c_str(), size1);
            assert(context.parse_request(&input, Timestamp::now()));
            assert(!context.got_all());

            size_t size2 = all.size() - size1;
            input.append(all.c_str() + size1, size2);
            assert(context.parse_request(&input, Timestamp::now()));
            assert(context.got_all());
            const HttpRequest &request = context.request();
            assert(request.method() == HttpRequest::k_get);
            assert(request.path() == string("/"));
            assert(request.get_version() == HttpRequest::k_http11);
            assert(request.get_header("Host") == string("code-david.cn"));
            assert(request.get_header("User-Agent") == string(""));
        }
    }

    // test empty header value
    {
        HttpContext context;
        Buffer input;
        input.append("GET / HTTP/1.1\r\n"
                    "Host: code-david.cn\r\n"
                    "User-Agent:\r\n"
                    "Accept-Encoding: \r\n"
                    "\r\n");
        assert(context.parse_request(&input, Timestamp::now()));
        assert(context.got_all());
        const HttpRequest &request = context.request();
        assert(request.method() == HttpRequest::k_get);
        assert(request.path() == string("/"));
        assert(request.get_version() == HttpRequest::k_http11);
        assert(request.get_header("Host") == string("code-david.cn"));
        assert(request.get_header("User-Agent") == string(""));
        assert(request.get_header("Accept-Encoding") == string(""));
    }
    printf("test finish successful\n");
}