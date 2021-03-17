/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "http/HttpContext.h"

#include "net/Buffer.h"

namespace web_server {

namespace http {

bool HttpContext::parse_request(Buffer *buf, Timestamp receive_time) {
    bool is_ok = true;
    bool has_more = true;
    while (has_more) {
        if (state_ == k_expect_request_line) {
            // 解析请求行
            const char *crlf = buf->find_CRLF(); // 查找"r\n"
            if (crlf) {
                // 查找成功，当前请求行完整
                is_ok = process_request_line(buf->peek(), crlf);
                if (is_ok) {
                    // 行解析成功
                    request_.set_receive_time(receive_time);
                    // 更新 Buffer
                    buf->retrieve_until(crlf + 2);
                    // 行解析成功，下一步是解析请求头
                    state_ = k_expect_headers;
                } else {
                    // 请求行解析失败
                    has_more = false;
                }
            } else {
                // 请求行不完整
                has_more = false;
            }
        } else if (state_ == k_expect_headers) {
            // 解析请求头
            const char *crlf = buf->find_CRLF(); // 查找"r\n"
            if (crlf) {
                // 查找成功，当前有一行完整的数据
                const char *colon = std::find(buf->peek(), crlf, ':'); // 查找":"
                if (colon != crlf) {
                    request_.addHeader(buf->peek(), colon, crlf);
                } else {
                    // 空行（"r\n"），请求头结束
                    state_ = k_got_all;
                    has_more = false;
                }
                // 更新 Buffer
                buf->retrieve_until(crlf + 2);
            } else {
                has_more = false;
            }
        } else if (state_ == k_expect_body) {
            // TODO
        }
    }
    return is_ok;
}

bool HttpContext::process_request_line(const char *start, const char *end) {
    bool is_succeed = false;
    // 请求方法 路径 HTTP/版本
    const char *space = std::find(start, end, ' ');
    if (space != end && request_.set_method(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            const char *question = std::find(start, space, '?');
            if (question != space) {
                request_.set_path(start, question);
                request_.set_query(question, space);
            } else {
                request_.set_path(start, space);
            }
        }
        start = space + 1;
        // 8个字节：HTTP/1.1
        is_succeed = end - start == 8 && std::equal(start, end-1, "HTTP/1.");
        if (is_succeed) {
            if (*(end - 1) == '1') {
                request_.set_version(HttpRequest::k_http11);
            } else if (*(end - 1) == '0') {
                request_.set_version(HttpRequest::k_http10);
            } else {
                is_succeed = false;
            }
        }
    }
    return is_succeed;
}

} // namespace http

} // namespace web_server