/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_HTTP_HTTPCONTEXT_H
#define WEB_SERVER_HTTP_HTTPCONTEXT_H

#include "base/Copyable.h"
#include "http/HttpRequest.h"
#include "net/Buffer.h"

namespace web_server {

namespace http {

using web_server::net::Buffer;

class HttpContext : public Copyable {
public:
    enum HttpRequestParseState {
        k_expect_request_line,
        k_expect_headers,
        k_expect_body,
        k_got_all,
    };

    HttpContext() :state_(k_expect_request_line) {
    }

    bool parse_request(Buffer *buf, Timestamp receive_time);

    bool got_all() const {
        return state_ == k_got_all;
    }

    void reset() {
        state_ = k_expect_request_line;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    const HttpRequest &request() const {
        return request_;
    }

    HttpRequest &request() {
        return request_;
    }

private:
    HttpRequestParseState state_;
    HttpRequest request_;

    bool process_request_line(const char *begin, const char *end);
};

} // namespace http

} // namespace web_server


#endif // WEB_SERVER_HTTP_HTTPCONTEXT_H