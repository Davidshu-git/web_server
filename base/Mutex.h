/**
 * @brief 线程同步中的互斥锁封装类，使用RAII机制封装了加锁解锁
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_BASE_MUTEX_H
#define WEB_SERVER_BASE_MUTEX_H

#include <pthread.h>
#include <cassert>

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

    /**
     * @brief 判断mutex是不是被当前线程拥有
     * 
     * @return true 
     * @return false 
     */
    bool is_locked_by_this_thread() const {
        return holder_ == current_thread::tid();
    }

    /**
     * @brief 断言该锁已经被当前线程锁住
     */
    void assert_locked() const {
        assert(is_locked_by_this_thread());
    }

    /**
     * @brief Get the pthread mutex object, this must not be const
     * @return pthread_mutex_t* 
     */
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
    /**
     * @brief 用于保存这个mutex归属的线程id
     */
    pid_t holder_;

    void unassign_holder() {
        holder_ = 0;
    }

    void assign_holder() {
        holder_ = current_thread::tid();
    }

    friend class Condition;

    /**
     * @brief 由于使用了private权限函数assign_holder等
     * 将其设置为内嵌类，使用RAII机制封装了assign_holder和unassign_holder
     */
    class UnassignGuard : private Noncopyable {
    public:
        explicit UnassignGuard(MutexLock& owner) : owner_(owner) {
        owner_.unassign_holder();
        }
        ~UnassignGuard() {
        owner_.assign_holder();
        }
    private:
        MutexLock& owner_;
  };
};

/**
 * @brief 利用RAII机制封装mutex类
 * 
 */
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
