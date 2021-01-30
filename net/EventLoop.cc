/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/EventLoop.h"

#include <sys/eventfd.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <vector>

using web_server::net::EventLoop;

namespace  {

__thread EventLoop *t_loop_in_this_thread = 0;

int create_event_fd() {
    int event_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(event_fd < 0) {
        abort();
    }
    return event_fd;
}

} // namespace 

namespace web_server {

namespace net {

EventLoop *EventLoop::get_event_loop_of_current_thread() {
    return t_loop_in_this_thread;
}

EventLoop::EventLoop() 
    : looping_(false),
      quit_(false),
      event_handleing_(false),
      calling_pending_functors_(false),
      iteration_(0),
      thread_ID_(current_thread::tid()),
      wakeup_fd_(create_event_fd()),
      mutex_() {
    t_loop_in_this_thread = this;
    // TODO
    // add wakeup channel
}

EventLoop::~EventLoop() {
    // TODO
    // add channel
    ::close(wakeup_fd_);
    t_loop_in_this_thread = NULL;
}

void EventLoop::loop() {
    assert(!looping_);
    assert_in_loop_thread();
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        ++iteration_;
        event_handleing_ = true;
        // TODO
        // add channel for loop
        event_handleing_ = false;
        do_pending_functors();
    }

    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!is_in_loop_thread()) {
        wakeup();
    }
}

void EventLoop::run_in_loop(Functor cb) {
    if (!is_in_loop_thread()) {
        cb();
    } else {
        queue_in_loop(cb);
    }
}

void EventLoop::queue_in_loop(Functor cb) {
    {
        MutexLockGuard lock(mutex_);
        pending_functors_.push_back(cb);
    }
    if (!is_in_loop_thread() || calling_pending_functors_) {
        wakeup();
    }
}

// TODO
// add time function
// add channel function

void EventLoop::abort_not_in_loop_thread() {
    printf("EventLoop thread_ID = %d, current thread id = %d\n", thread_ID_, current_thread::tid());
}

void EventLoop::wakeup() {
    printf("add socker operation\n");
}

void EventLoop::handle_read() {
    printf("add socker operation\n");
}

void EventLoop::do_pending_functors() {
    std::vector<Functor> functors;
    calling_pending_functors_ = true;
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pending_functors_);
    }
    for(const auto &functor : functors) {
        functor();
    }
    calling_pending_functors_ = false;
}

} // namespace net

} // namespace web_server