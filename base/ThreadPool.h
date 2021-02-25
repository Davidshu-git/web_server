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

class ThreadPool : private Noncopyable {
public:
    typedef std::function<void ()> Task;
    explicit ThreadPool(const std::string& nameArg = std::string("ThreadPool"));
    ~ThreadPool();

    void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
    void setThreadInitCallback(const Task& cb) { threadInitCallback_ = cb; }

    void start(int numThreads);
    void stop();

    const std::string& name() const { return name_; }
    size_t queueSize() const;

    void run(Task f);

private:
    bool isFull() const;
    void runInThread();
    Task take();

    mutable MutexLock mutex_;
    Condition notEmpty_;
    Condition notFull_;
    std::string name_;
    Task threadInitCallback_;
    std::vector<std::unique_ptr<web_server::Thread>> threads_;
    std::deque<Task> queue_;
    size_t maxQueueSize_;
    bool running_;
};

} // namespace web_server


#endif // WEB_SERVER_BASE_THREADPOOL_H