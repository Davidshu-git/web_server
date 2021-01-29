/**
 * @brief Mutex wrapper
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_BASE_MUTEX_H
#define WEB_SERVER_BASE_MUTEX_H

#include <pthread.h>
#include <assert.h>

#include "base/Noncopyable.h"
#include "base/CurrentThread.h"

namespace web_server {

/**
 * @brief Mutex lock class
 * 
 */
class MutexLock : private Noncopyable {
public:
    MutexLock() : holder_(0) {
        pthread_mutex_init(&mutex_, NULL);
    }
    ~MutexLock() {
        assert(holder_ == 0);
        pthread_mutex_destroy(&mutex_);
    }

    bool is_locked_by_this_thread() const {
        return holder_ == current_thread::tid();
    }

    void assert_locked() const {
        assert(is_locked_by_this_thread());
    }

    pthread_mutex_t *get_pthread_mutex() {
        return &mutex_;
    }

    void lock() {
        pthread_mutex_lock(&mutex_);
        assign_holder();
    }

    void unlock() {
        unassign_holder();
        pthread_mutex_unlock(&mutex_);
    }

private:
    pthread_mutex_t mutex_;
    pid_t holder_;

    void unassign_holder() {
        holder_ = 0;
    }

    void assign_holder() {
        holder_ = current_thread::tid();
    }

    // TODO
    // UnassignGuard class
};

class MutexLockGuard : private Noncopyable {
public:
    explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) {
        mutex_.lock();
    }
    ~MutexLockGuard() {
        mutex_.unlock();
    }
private:
    MutexLock& mutex_;
};

} // namespace web_server

#endif // WEB_SERVER_BASE_MUTEX_H
