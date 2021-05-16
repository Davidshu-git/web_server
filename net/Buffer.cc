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
 * @brief 为了保证一次性读完，而不是由于受到buffer空间所限要分几次进行读取
 * 默认epoll采用LT模式，只要socket的接收缓冲区有数据，那么会不停触发事件
 * 所以最好一次性将数据全读干净，避免多次调用read
 * @param fd 
 * @param saved_errno 传出参数
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

    // 当可写入空间小于extra_buf的大小时，说明空间不太够，需要用到extr_buf
    // 使用readv一次性将数据分散读到对应的两个空间去
    const int iovcnt = (writable < sizeof extra_buf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);

    if (n < 0) {
        *saved_errno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        // 若读到的所有数据大小小于等于可写入空间，则说明只用到了buffer自己的空间
        writer_index_ += n;
    } else {
        // 若大于可写入空间，则说明buffer已经写满了，剩余部分写到了extra_buf中
        // 此时需要将extra_buf中的数据放到buffer中去
        writer_index_ = buffer_.size();
        append(extra_buf, n - writable);
    }
    return n;
}

} // namespace net

} // namespace web_server