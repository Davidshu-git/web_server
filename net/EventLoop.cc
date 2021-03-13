/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/EventLoop.h"

#include <sys/eventfd.h>
#include <cstdlib>
#include <unistd.h>

#include <cassert>
#include <vector>
#include <algorithm>
#include <functional>

#include "base/Logging.h"
#include "net/Channel.h"
#include "net/Poller.h"
#include "net/TimerQueue.h"

using web_server::net::EventLoop;

/**
 * @brief 使用未命名的命名空间
 * 具有静态全局意义，但仅在该文件内有效
 */
namespace  {

__thread EventLoop *t_loop_in_this_thread = 0;

const int kPollTimeMs = 10000;

int create_event_fd() {
    int event_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(event_fd < 0) {
        LOG_SYSERR << "Failed in eventfd";
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

/**
 * @brief Construct a new Event Loop:: Event Loop object
 * 保证单一线程中仅有一个EventLoop对象存在
 */
EventLoop::EventLoop() 
    : looping_(false),
      quit_(false),
      event_handling_(false),
      calling_pending_functors_(false),
      iteration_(0),
      thread_ID_(current_thread::tid()),
      poller_(Poller::new_default_poller(this)),
      wakeup_fd_(create_event_fd()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      current_active_channel_(NULL) {
    LOG_DEBUG << "EventLoop created " << this << " in thread " << thread_ID_;
    if (t_loop_in_this_thread) {
        LOG_FATAL << "Another EventLoop " << t_loop_in_this_thread 
                  << " exists in this thread " << thread_ID_;
    } else {
        t_loop_in_this_thread = this;
    }
    wakeup_channel_->set_read_callback(std::bind(&EventLoop::handle_read, this));
    wakeup_channel_->enable_reading();
}

EventLoop::~EventLoop() {
    LOG_DEBUG << "EventLoop " << this << " of thread " << thread_ID_
              << " destructs in thread " << current_thread::tid();
    wakeup_channel_->disable_all();
    wakeup_channel_->remove();
    ::close(wakeup_fd_);
    t_loop_in_this_thread = NULL;
}

void EventLoop::loop() {
    assert(!looping_);
    assert_in_loop_thread();
    looping_ = true;
    quit_ = false;
    LOG_TRACE << "EventLoop " << this << " start looping";

    while (!quit_) {
        active_channels_.clear();
        poll_return_time_ = poller_->poll(kPollTimeMs, &active_channels_);
        ++iteration_;
        if (Logger::log_level() <= Logger::TRACE) {
            print_active_channels();
        }
        event_handling_ = true;
        for (Channel *channel : active_channels_) {
            current_active_channel_ = channel;
            current_active_channel_->handle_event(poll_return_time_);
        }
        current_active_channel_ = NULL;
        event_handling_ = false;
        do_pending_functors();
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!is_in_loop_thread()) {
        wakeup();
    }
}

void EventLoop::run_in_loop(Functor cb) {
    if (is_in_loop_thread()) {
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

TimerID EventLoop::run_at(Timestamp time, TimerCallback cb) {
    return timer_queue_->add_timer(cb, time, 0.0);
}

TimerID EventLoop::run_after(double delay, TimerCallback cb) {
    Timestamp time(add_time(Timestamp::now(), delay));
    return run_at(time, cb);
}

TimerID EventLoop::run_every(double interval, TimerCallback cb) {
    Timestamp time(add_time(Timestamp::now(), interval));
    return timer_queue_->add_timer(cb, time, interval);
}

void EventLoop::cancel(TimerID timer_ID) {
    return timer_queue_->cancel(timer_ID);
}

void EventLoop::update_channel(Channel *channel) {
    assert(channel->owner_loop() == this);
    assert_in_loop_thread();
    poller_->update_channel(channel);
}

void EventLoop::remove_channel(Channel *channel) {
    assert(channel->owner_loop() == this);
    assert_in_loop_thread();
    if (event_handling_) {
        assert(current_active_channel_ == channel ||
            std::find(active_channels_.begin(), active_channels_.end(), channel) == active_channels_.end());
    }
    poller_->remove_channel(channel);
}

bool EventLoop::has_channel(Channel *channel) {
    assert(channel->owner_loop() == this);
    assert_in_loop_thread();
    return poller_->has_channel(channel);
}

void EventLoop::abort_not_in_loop_thread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << thread_ID_
              << ", current thread id = " <<  current_thread::tid();
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeup_fd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handle_read() {
    uint64_t one = 1;
    ssize_t n = read(wakeup_fd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
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

void EventLoop::print_active_channels() const {
    for (Channel *channel : active_channels_) {
        LOG_TRACE << "{" << channel->revents_to_string() << "} ";
    }
}

} // namespace net

} // namespace web_server