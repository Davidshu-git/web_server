/**
 * @brief current thread which will be used
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_BASE_CURRENTTHREAD_H
#define WEB_SERVER_BASE_CURRENTTHREAD_H

// Optimizing compiler GCC buildin function
#define likely(x) __builtin_expect(!!(x), 1) // x more likely be true
#define unlikely(x) __builtin_expect(!!(x), 0) // x more likely be false

#include <cstdint>

namespace web_server {

namespace current_thread {

extern __thread int t_cached_tid;
extern __thread char t_tid_string[32];
extern __thread int t_tid_string_length;
extern __thread const char *t_thread_name;

void cached_tid();

bool is_main_thread();

void sleep_usec(int64_t usec);

/**
 * @brief 使用封装好的函数获取系统tid信息
 * 调用此函数将执行将tid信息缓存到当前线程的任务
 * 
 * @return int 
 */
inline int tid() {
    if(unlikely(t_cached_tid == 0)) {
        cached_tid();
    }
    return t_cached_tid;
}

inline const char *tid_string() {
    return t_tid_string;
}

inline int tid_string_length() {
    return t_tid_string_length;
}

inline const char *thread_name() {
    return t_thread_name;
}

// TODO
// add sleep_u_sec function
// add stack_trace function

} // namespace current_thread

} // namespace web_server

#endif // WEB_SERVER_BASE_CURRENTTHREAD_H