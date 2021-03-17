/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/Buffer.h"

#include <cerrno>
#include <sys/uio.h>

namespace web_server {

namespace net {

const char Buffer::k_CRLF[] = "\r\n";

const size_t Buffer::k_cheap_prepend;
const size_t Buffer::k_initial_size;

/**
 * @brief 
 * 
 * @param fd 
 * @param saved_errno 
 * @return ssize_t 
 */
ssize_t Buffer::read_fd(int fd, int *saved_errno) {
    char extra_buf[65536];
    struct iovec vec[2];
    const size_t writable = writable_bytes();
    vec[0].iov_base = begin() + writer_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extra_buf;
    vec[1].iov_len = sizeof extra_buf;

    const int iovcnt = (writable < sizeof extra_buf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);

    if (n < 0) {
        *saved_errno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        writer_index_ += n;
    } else {
        writer_index_ = buffer_.size();
        append(extra_buf, n - writable);
    }
    return n;
}

} // namespace net

} // namespace web_server