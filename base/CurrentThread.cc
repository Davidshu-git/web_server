/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/CurrentThread.h"
#include "base/Timestamp.h"

#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>

namespace web_server {

namespace detail{

/**
 * @brief 用于不同进程间的线程通讯
 * 获得线程实际的进程id
 * 由于glibc没实现这个gettid
 * 需要利用syscall
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
 */
void cached_tid() {
    if(t_cached_tid == 0) {
        t_cached_tid = detail::gettid();
        t_tid_string_length = snprintf(t_tid_string, sizeof(t_tid_string),
            "%5d ", t_cached_tid);
    }
}

/**
 * @brief 判断是不是主线程的方法就是将该线程进程id与当前pid比较
 * 若相同则为主线程
 * @return true 
 * @return false 
 */
bool is_main_thread() {
    return tid() == ::getpid();
}

/**
 * @brief 短延迟函数，停止执行一段微秒为单位的时间
 * @param usec 微秒
 */
void sleep_usec(int64_t usec) {
    struct timespec ts = {0, 0};
    ts.tv_sec = static_cast<time_t>(usec / Timestamp::k_micro_seconds_per_second);
    ts.tv_nsec = static_cast<long>(usec % Timestamp::k_micro_seconds_per_second * 1000);
    ::nanosleep(&ts, NULL);
}

} // namespace current_thread

} // namespace web_server