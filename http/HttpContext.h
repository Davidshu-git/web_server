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

/**
 * @brief 负责解析http文本的工作
 * 
 */
class HttpContext : public Copyable {
public:
    enum HttpRequestParseState {
        k_expect_request_line,      // 解析请求行
        k_expect_headers,           // 解析请求首部或状态首部
        k_expect_body,              // 解析响应体
        k_got_all,                  // 解析完成
    };

    /**
     * @brief Construct a new Http Context object
     * 初始从解析请求行或状态行开始
     */
    HttpContext() :state_(k_expect_request_line) {}

    /**
     * @brief 解析buffer中的数据，将数据保存到request中
     * 
     * @param buf 
     * @param receive_time 
     * @return true 
     * @return false 
     */
    bool parse_request(Buffer *buf, Timestamp receive_time);

    bool got_all() const {
        return state_ == k_got_all;
    }

    /**
     * @brief 清空HttpRequest对象
     * 
     */
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