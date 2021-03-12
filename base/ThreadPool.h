/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_BASE_THREADPOOL_H
#define WEB_SERVER_BASE_THREADPOOL_H

#include <vector>
#include <functional>
#include <memory>
#include <deque>
#include <string>

#include "base/Noncopyable.h"
#include "base/Mutex.h"
#include "base/Condition.h"
#include "base/Thread.h"

namespace web_server {

/**
 * @brief 线程池对象管理一个线程池
 * 
 */
class ThreadPool : private Noncopyable {
public:
    typedef std::function<void()> Task;
    explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
    ~ThreadPool();

    void set_max_queue_size(int max_queue_size) { max_queue_size_ = max_queue_size; }
    void set_thread_init_callback(const Task& cb) { thread_init_callback_ = cb; }

    void start(int num_threads);
    void stop();
    void run(Task f);

    const std::string& name() const { 
        return name_; 
    }

    size_t queue_size() const {
        MutexLockGuard lock(mutex_);
        return task_queue_.size();
    }

private:
    mutable MutexLock mutex_;
    Condition not_empty_;
    Condition not_full_;
    std::string name_;
    Task thread_init_callback_;
    /**
     * @brief 通过uniqu_ptr管理thread对象
     */
    std::vector<std::unique_ptr<web_server::Thread>> threads_;
    std::deque<Task> task_queue_;
    size_t max_queue_size_;
    bool running_;

    bool is_full() const {
        mutex_.assert_locked();
        return max_queue_size_ > 0 && task_queue_.size() >= max_queue_size_;
    }

    void run_in_thread();
    Task take();
};

} // namespace web_server


#endif // WEB_SERVER_BASE_THREADPOOL_H