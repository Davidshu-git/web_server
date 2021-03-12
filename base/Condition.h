/**
 * @brief 封装了线程同步的条件变量类
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

    /**
     * @brief 等待时，对于pthrea_cond_wait而言会执行解锁操作
     * 所以此时既然解锁，那么该线程就不拥有锁了
     * 对于这个锁就需要unassign_holder
     * 但收到通知后会进行加锁，此时又要assign_holder
     * 利用RAII机制的guard可实现自动的assign和unassign
     */
    void wait() {
        MutexLock::UnassignGuard ug(mutex_);
        pthread_cond_wait(&pcond_, mutex_.get_pthread_mutex());
    }

    /**
     * @brief 通知一个处于wait阻塞的线程
     */
    void notify() {
        pthread_cond_signal(&pcond_);
    }

    /**
     * @brief 通知所有处于wait阻塞的线程
     * 
     */
    void notify_all() {
        pthread_cond_broadcast(&pcond_);
    }

    // bool wait_for_seconds(double seconds);
private:
    MutexLock &mutex_;
    pthread_cond_t pcond_;
};

} // namespace web_server

#endif // WEB_SERVER_BASE_CONDITION_H