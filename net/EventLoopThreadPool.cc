/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/EventLoopThreadPool.h"

#include <cassert>
#include <string>

#include "net/EventLoop.h"
#include "net/EventLoopThread.h"

namespace web_server {

namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop, const std::string &name)
    : base_loop_(base_loop),
      name_(name),
      started_(false),
      num_threads_(0),
      next_(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
    assert(!started_);
    base_loop_->assert_in_loop_thread();

    started_ = true;
    for (int i = 0; i < num_threads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->start_loop());
    }
    if (num_threads_ == 0 && cb) {
        cb(base_loop_);
    }
}

/**
 * @brief 每次新建一个tcpconnection都会调用该函数获得一个loop来管理这个tcpconnection
 * 若loops_为空，则使用的是base_loop，若不为空，则轮流使用loops_中的loop
 * @return EventLoop* 
 */
EventLoop *EventLoopThreadPool::get_next_loop() {
    base_loop_->assert_in_loop_thread();
    assert(started_);
    EventLoop *loop = base_loop_;
    if (!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if (static_cast<decltype(loops_.size())>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

EventLoop *EventLoopThreadPool::get_loop_from_hash(size_t hash_code) {
    base_loop_->assert_in_loop_thread();
    EventLoop *loop = base_loop_;

    if (!loops_.empty()) {
        loop = loops_[hash_code % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::get_all_loops() {
    base_loop_->assert_in_loop_thread();
    assert(started_);
    if (loops_.empty()) {
        return std::vector<EventLoop *>(1, base_loop_);
    } else {
        return loops_;
    }
}

} // namespace net

} // namespace web_server