/**
 * @brief countdown latch
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_BASE_COUNTDOWNLATCH_H
#define WEB_SERVER_BASE_COUNTDOWNLATCH_H

#include "base/Noncopyable.h"
#include "base/Mutex.h"
#include "base/Condition.h"

namespace web_server {

class CountDownLatch : private Noncopyable {
public:
    CountDownLatch(int count) : mutex_(), condition_(mutex_), count_(count) {}

    int get_count() const {
        MutexLockGuard lock(mutex_);
        return count_;
    }

    void wait();

    void count_down();

private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};

} // namespace web_server


#endif // WEB_SERVER_BASE_COUNTDOWNLATCH_H