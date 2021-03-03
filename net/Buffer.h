/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_BUFFER_H
#define WEB_SERVER_NET_BUFFER_H

#include "base/Copyable.h"

#include <vector>
#include <cassert>
#include <algorithm>
#include <cstring>
#include <string>

namespace web_server {

namespace net {

class Buffer : private Copyable {
public:
    static const size_t k_cheap_prepend = 8;
    static const size_t k_initial_size = 1024;
    
    explicit Buffer(size_t initial_size = k_initial_size) 
        : buffer_(k_cheap_prepend + initial_size),
          reader_index_(k_cheap_prepend),
          writer_index_(k_cheap_prepend){
        assert(readable_bytes() == 0);
        assert(writable_bytes() == initial_size);
        assert(prependable_bytes() == k_cheap_prepend);
    }

    size_t readable_bytes() const {
        return writer_index_ - reader_index_;
    }

    size_t writable_bytes() const {
        return buffer_.size() - writer_index_;
    }

    size_t prependable_bytes() const {
        return reader_index_;
    }

    void swap(Buffer &rhs) {
        buffer_.swap(rhs.buffer_);
        std::swap(reader_index_, rhs.reader_index_);
        std::swap(writer_index_, rhs.writer_index_);
    }

    const char *peek() const {
        return begin() + reader_index_;
    }

    char *begin_write() {
        return begin() + writer_index_;
    }

    const char *begin_write() const {
        return begin() + writer_index_;
    }

    const char *find_CRLF() const {
        const char *crlf = std::search(peek(), begin_write(), k_CRLF, k_CRLF + 2);
        return crlf == begin_write() ? NULL : crlf;
    }

    const char *find_CRLF(const char *start) const {
        assert(peek() <= start);
        assert(start <= begin_write());
        const char *crlf = std::search(start, begin_write(), k_CRLF, k_CRLF + 2);
        return crlf == begin_write() ? NULL : crlf;
    }

    const char *fing_EOL() const {
        const void *eol = memchr(peek(), '\n', readable_bytes());
        return static_cast<const char *>(eol);
    }
    
    const char *find_EOL(const char *start) const {
        assert(peek() <= start);
        assert(start <= begin_write());
        const void *eol = memchr(start, '\n', begin_write() - start);
        return static_cast<const char *>(eol);
    }

    void retrieve_all() {
        reader_index_ = k_cheap_prepend;
        writer_index_ = k_cheap_prepend;
    }

    void retrieve(size_t len) {
        assert(len <= readable_bytes());
        if (len < readable_bytes()) {
            reader_index_ +=len;
        } else {
            retrieve_all();
        }
    }

    void retrieve_until(const char *end) {
        assert(peek() <= end);
        assert(end <= begin_write());
        retrieve(end - peek());
    }

    void retrieve_int64() {
        retrieve(sizeof(int64_t));
    }

    void retrieve_int32() {
        retrieve(sizeof(int32_t));
    }

    void retrieve_int16() {
        retrieve(sizeof(int16_t));
    }

    void retrieve_int8() {
        retrieve(sizeof(int8_t));
    }

    std::string retrieve_as_string(size_t len) {
        assert(len <= readable_bytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    std::string retrieve_all_as_string() {

    }

private:
    std::vector<char> buffer_;
    size_t reader_index_;
    size_t writer_index_;

    static const char k_CRLF[];

    char *begin() {
        return &*buffer_.begin();
    }

    const char *begin() const{
        return &*buffer_.begin();
    }

    void make_space(size_t len) {
        if (writable_bytes() + prependable_bytes() < len + k_cheap_prepend) {
            buffer_.resize(writer_index_ + len);
        } else {
            assert(k_cheap_prepend < reader_index_);
            size_t readable = readable_bytes();
            std::copy(begin() + reader_index_,
                      begin() + writer_index_,
                      begin() + k_cheap_prepend);
            reader_index_ = k_cheap_prepend;
            writer_index_ = reader_index_ + readable;
            assert(readable == readable_bytes());
        }
    }
};

} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_BUFFER_H