/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_HTTP_HTTPRESPONSE_H
#define WEB_SERVER_HTTP_HTTPRESPONSE_H

#include <map>
#include <string>

#include "base/Copyable.h"
#include "net/Buffer.h"

namespace web_server {

namespace http {

using web_server::net::Buffer;

/**
 * @brief 负责管理http响应报文中的信息
 * 
 */
class HttpResponse : public Copyable {
public:
    enum HttpStatusCode {
        k_unknown,
        k_200_ok = 200,
        k_301_moved_permanently = 301,
        k_400_bad_request = 400,
        k_404_not_found = 404
    };

    explicit HttpResponse(bool close) 
    : status_code_(k_unknown),
      close_connection_(close) {
    }

    void set_status_code(HttpStatusCode code) {
        status_code_ = code;
    }

    void set_status_message(const std::string &message) {
        status_message_ = message;
    }

    void set_close_connection(bool on) {
        close_connection_ = on;
    }

    bool close_connection() const {
        return close_connection_;
    }

    void set_content_type(const std::string &content_type) {
        add_header("Content-Type", content_type);
    }

    void add_header(const std::string &key, const std::string &value) {
        headers_[key] = value;
    }

    void set_body(const std::string &body) {
        body_ = body;
    }
    
    /**
     * @brief 将响应报文数据存放到buffer中
     * 
     * @param output 
     */
    void append_to_buffer(Buffer *output) const;
    
private:
    std::map<std::string, std::string> headers_;    // 存放响应首部字段
    HttpStatusCode status_code_;                    // 存放状态码
    std::string status_message_;                    // 存放状态信息
    bool close_connection_;                         // 是否设置Connection字段为close
    std::string body_;                              // 存放响应体
};

} // namespace http

} // namespace web_server

#endif // WEB_SERVER_HTTP_HTTPRESPONSE_H