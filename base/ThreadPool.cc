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

ThreadPool::ThreadPool(const std::string& nameArg)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      name_(nameArg),
      maxQueueSize_(0),
      running_(false) {}

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
}

void ThreadPool::start(int numThreads) {
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        threads_.emplace_back(new web_server::Thread(
            std::bind(&ThreadPool::runInThread, this)));
        threads_[i]->start();
    }
    if (numThreads == 0 && threadInitCallback_) {
        threadInitCallback_();
    }
}

void ThreadPool::stop() {
    {
    MutexLockGuard lock(mutex_);
    running_ = false;
    notEmpty_.notify_all();
    notFull_.notify_all();
    }
    for (auto& thr : threads_) {
        thr->join();
    }
}

size_t ThreadPool::queueSize() const {
  MutexLockGuard lock(mutex_);
  return queue_.size();
}

void ThreadPool::run(Task task) {
    if (threads_.empty()){
        task();
    } else {
        MutexLockGuard lock(mutex_);
        while (isFull() && running_) {
            notFull_.wait();
        }
        if (!running_) return;
        assert(!isFull());
        queue_.push_back(std::move(task));
        notEmpty_.notify();
    }
}

ThreadPool::Task ThreadPool::take() {
    MutexLockGuard lock(mutex_);
    while (queue_.empty() && running_) {
        notEmpty_.wait();
    }
    Task task;
    if (!queue_.empty()) {
        task = queue_.front();
        queue_.pop_front();
        if (maxQueueSize_ > 0) {
            notFull_.notify();
        }
    }
    return task;
}

bool ThreadPool::isFull() const {
    mutex_.assert_locked();
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread() {
    if (threadInitCallback_) {
      threadInitCallback_();
    }
    while (running_) {
      Task task(take());
      if (task) {
        task();
      }
    }
}

} // namespace web_server