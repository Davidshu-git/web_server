/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/EventLoopThread.h"

#include "net/EventLoop.h"

namespace web_server {

namespace net {

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
                                 const std::string &name)
    : loop_(NULL),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::thread_func, this), name),
      mutex_(),
      cond_(mutex_),
      callback_(cb) {
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != NULL) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::start_loop() {
    assert(!thread_.started());
    thread_.start();

    EventLoop *loop = NULL;
    {
        MutexLockGuard lock(mutex_);
        while (loop_ == NULL) {
            cond_.wait();
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::thread_func() {
    EventLoop loop;
    if (callback_) {
        callback_(&loop);
    }
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();
    MutexLockGuard lock(mutex_);
    loop_ = NULL;
}

} // namespace net

} // namespace web_server