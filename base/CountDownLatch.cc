/**
 * @brief countdown latch source file
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/CountDownLatch.h"

namespace web_server {

/**
 * @brief 由于要访问count_值，若同时被多个线程访问同一个计数锁对象
 * 产生了竞态，此时是非线程安全的，需要使用锁保护
 */
void CountDownLatch::wait() {
    MutexLockGuard lock(mutex_);
    while (count_ > 0) {
        condition_.wait();
    }
}

/**
 * @brief 修改count_需要加锁
 * 若计数为0，通知所有wait线程
 */
void CountDownLatch::count_down() {
    MutexLockGuard lock(mutex_);
    --count_;
    if(count_ == 0) {
        condition_.notify_all();
    }
}

} // namespace web_server