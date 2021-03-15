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

/**
 * @brief 真正执行thread创建工作
 * 获取创建后端eventloo指针，需要是要条件变量机制等待
 * @return EventLoop* 
 */
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

/**
 * @brief 执行创建eventloop工作
 * 执行callback_
 * 若是创建成功，通过条件变量通知创建线程，这个线程创建好了
 * 通知其取eventloop的指针值
 */
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