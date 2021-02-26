/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/CurrentThread.h"

#include <sys/syscall.h>
#include <unistd.h>
#include <cstdio>

namespace web_server {

namespace detail{

/**
 * @brief 用于不同进程间的线程通讯，由于glibc没实现这个gettid，需要利用syscall
 * 
 * @return pid_t 
 */
pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

} // namespace detail


namespace current_thread {

__thread int t_cached_tid = 0;
__thread char t_tid_string[32];
__thread int t_tid_string_length = 6;
__thread const char *t_thread_name = "unknown";

/**
 * @brief 将系统调用中得到的tid信息缓存到当前线程存储设施中
 * 该设施以关键字__thread修饰
 * 
 */
void cached_tid() {
    if(t_cached_tid == 0) {
        t_cached_tid = detail::gettid();
        t_tid_string_length = snprintf(t_tid_string, sizeof(t_tid_string),
            "%5d ", t_cached_tid);
    }
}

bool is_main_thread() {
    return tid() == ::getpid();
}

} // namespace current_thread

} // namespace web_server