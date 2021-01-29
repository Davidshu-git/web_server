/**
 * @brief condition
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_BASE_CONDITION_H
#define WEB_SERVER_BASE_CONDITION_H

#include "base/Noncopyable.h"
#include "base/Mutex.h"

namespace web_server {

class Condition : private Noncopyable {
public:
    explicit Condition(MutexLock& mutex) : mutex_(mutex) {
        pthread_cond_init(&pcond_, NULL);
    }
    ~Condition() {
        pthread_cond_destroy(&pcond_);
    }

    void wait() {
        // TODO
        // add UnassignGuard class
        pthread_cond_wait(&pcond_, mutex_.get_pthread_mutex());
    }

    void notify() {
        pthread_cond_signal(&pcond_);
    }

    void notify_all() {
        pthread_cond_broadcast(&pcond_);
    }

    // TODO
    // add time out function
private:
    MutexLock &mutex_;
    pthread_cond_t pcond_;
};

} // namespace web_server

#endif // WEB_SERVER_BASE_CONDITION_H