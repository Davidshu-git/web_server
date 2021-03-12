/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/ThreadPool.h"

#include <cassert>
#include <cstdio>
#include <string>

#include "base/Logging.h"

namespace web_server {

/**
 * @brief Construct a new Thread Pool:: Thread Pool
 * 仅创建了Mutex对象，给线程池name赋值
 * @param name 
 */
ThreadPool::ThreadPool(const std::string& name)
    : mutex_(),
      not_empty_(mutex_),
      not_full_(mutex_),
      name_(name),
      max_queue_size_(0),
      running_(false) {
}

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
}

/**
 * @brief 根据设定对num_threads个数创建线程thread对象
 * 启动这些线程对象，将这些线程对象的unique_ptr存放到threads_中
 * @param num_threads 
 */
void ThreadPool::start(int num_threads) {
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        char id[32];
        snprintf(id, sizeof id, "%d", i+1);
        threads_.emplace_back(new web_server::Thread(
            std::bind(&ThreadPool::run_in_thread, this), name_+id));
        threads_[i]->start();
    }
    if (num_threads == 0 && thread_init_callback_) {
        thread_init_callback_();
    }
}

/**
 * @brief 停止时需要对所有阻塞在wait的条件变量进行通知
 * 分别回收线程池中的所有线程对象，调用thread的join方法即可
 */
void ThreadPool::stop() {
    {
    MutexLockGuard lock(mutex_);
    running_ = false;
    not_empty_.notify_all();
    not_full_.notify_all();
    }
    for (auto& thr : threads_) {
        thr->join();
    }
}

/**
 * @brief 对共享变量中max_queue_size_、task_queue_、running_需要进行访问
 * 此时要加锁保护，进入wait()阻塞
 * 该函数木的是向线程池对象中的任务队列添加任务
 * @param task 
 */
void ThreadPool::run(Task task) {
    if (threads_.empty()){
        task();
    } else {
        MutexLockGuard lock(mutex_);
        while (is_full() && running_) {
            not_full_.wait();
        }
        if (!running_) return;
        assert(!is_full());
        task_queue_.push_back(task);
        not_empty_.notify();
    }
}

/**
 * @brief 线程thread对象中需要执行的任务
 * 不停取任务，由于传递可调用对象时传递的是类成员对象，this指针也被传递了
 * 所以在子线程中可以访问thread_pool对象中的成员变量running_
 * 仅仅是判断running_并未对其做出修改，可不用加锁
 */
void ThreadPool::run_in_thread() {
    if (thread_init_callback_) {
        thread_init_callback_();
    }
    while (running_) {
        Task task(take());
        if (task) {
            task();
      }
    }
}

/**
 * @brief 取任务时，由于是对共享变量进行了操作
 * 要加锁保护任务队列
 * @return ThreadPool::Task 
 */
ThreadPool::Task ThreadPool::take() {
    MutexLockGuard lock(mutex_);
    while (task_queue_.empty() && running_) {
        not_empty_.wait();
    }
    Task task;
    if (!task_queue_.empty()) {
        task = task_queue_.front();
        task_queue_.pop_front();
        if (max_queue_size_ > 0) {
            not_full_.notify();
        }
    }
    return task;
}

} // namespace web_server