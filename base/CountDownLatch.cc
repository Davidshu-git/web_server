/**
 * @brief countdown latch source file
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/CountDownLatch.h"

namespace web_server {

void CountDownLatch::wait() {
    MutexLockGuard lock(mutex_);
    while (count_ > 0) {
        condition_.wait();
    }
}

void CountDownLatch::count_down() {
    MutexLockGuard lock(mutex_);
    --count_;
    if(count_ == 0) {
        condition_.notify_all();
    }
}

} // namespace web_server