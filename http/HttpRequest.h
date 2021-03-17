/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_HTTP_HTTPREQUEST_H
#define WEB_SERVER_HTTP_HTTPREQUEST_H

#include "base/Copyable.h"

#include <string>
#include <map>
#include <cassert>

#include "base/Timestamp.h"

namespace web_server {

namespace http {

class HttpRequest : public Copyable {
public:
    enum Method {k_invalid, k_get, k_post, k_head, k_put, k_delete};
    enum Version {k_unknown, k_http10, k_http11};

    HttpRequest() : method_(k_invalid), version_(k_unknown) {
    }

    void set_version(Version v) {
        version_ = v;
    }

    Version get_version() const {
        return version_;
    }

    bool set_method(const char *start, const char *end) {
        assert(method_ == k_invalid);
        std::string m(start, end);
        if (m == "GET") {
            method_ = k_get;
        } else if (m == "POST") {
            method_ = k_post;
        } else if (m == "HEAD") {
            method_ = k_head;
        } else if (m == "PUT") {
            method_ = k_put;
        } else if (m == "DELETE") {
            method_ = k_delete;
        } else {
            method_ = k_invalid;
        }
        return method_ != k_invalid;
    }

    Method method() const {
        return method_;
    }

    const char *method_string() const {
        const char *m_str = "UNKNOWN";
        switch (method_) {
            case k_get:
                m_str = "GET";
                break;
            case k_post:
                m_str = "POST";
                break;
            case k_head:
                m_str = "HEAD";
                break;
            case k_put:
                m_str = "PUT";
                break;
            case k_delete:
                m_str = "DELETE";
                break;
            default:
                break;
        }
        return m_str;
    }

    void set_path(const char *start, const char *end) {
        path_.assign(start, end);
    }

    const std::string &path() const {
        return path_;
    }

    void set_query(const char *start, const char *end) {
        query_.assign(start, end);
    }

    const std::string &query() const {
        return query_;
    }

    void set_receive_time(Timestamp time) {
        receive_time_ = time;
    }

    Timestamp receive_time() const {
        return receive_time_;
    }

    void addHeader(const char *start, const char *colon, const char *end) {
        std::string field(start, colon);
        ++colon;

        // 跳过开头空格
        while (colon < end && ::isspace(*colon)) {
            ++colon;
        }
        // 跳过尾部空格
        std::string value(colon, end);
        while (!value.empty() && isspace(value[value.size() - 1])) {
            value.resize(value.size() - 1);
        }
        headers_[field] = value;
    }

    std::string getHeader(const std::string &field) const {
        std::string value;
        auto it = headers_.find(field);
        if (it != headers_.end()) {
            value = it->second;
        }
        return value;
    }

    const std::map<std::string, std::string> &headers() const {
        return headers_;
    }

    void swap(HttpRequest &that) {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        receive_time_.swap(that.receive_time_);
        headers_.swap(that.headers_);
    }
private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    Timestamp receive_time_;
    std::map<std::string, std::string> headers_;
};

} // namespace http

} // namespace web_server

#endif // WEB_SERVER_HTTP_HTTPREQUEST_H