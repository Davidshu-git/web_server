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

    void append_to_buffer(Buffer *output) const;
    
private:
    std::map<std::string, std::string> headers_;
    HttpStatusCode status_code_;
    std::string status_message_;
    bool close_connection_;
    std::string body_;
};

} // namespace http

} // namespace web_server

#endif // WEB_SERVER_HTTP_HTTPRESPONSE_H