/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "http/HttpResponse.h"

namespace web_server {

namespace http {

void HttpResponse::append_to_buffer(Buffer *output) const {
    // std::string buf = "HTTP/1.1 %d ";
    // buf += std::to_string(status_code_);
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", status_code_);
    output->append(buf);
    output->append(status_message_);
    output->append("\r\n");

    if (close_connection_) {

        output->append("Connection: close\r\n");
    } else {
        snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
        output->append(buf);
        output->append("Connection: Keep-Alive\r\n");
    }

    for (const auto &header : headers_) {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }

    output->append("\r\n");
    output->append(body_);
}

} // namespace http

} // namespace web_server