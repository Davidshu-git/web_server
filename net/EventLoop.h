/**
 * @brief Public header file, and it must only include public header file
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_EVENTLOOP_H
#define WEB_SERVER_NET_EVENTLOOP_H

#include <functional>
#include <atomic>
#include <vector>

#include "base/Noncopyable.h"
#include "base/Mutex.h"
#include "base/CurrentThread.h"

namespace web_server {

namespace net {

class EventLoop : private Noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    int64_t iteration() const {
        return iteration_;
    }

    bool is_in_loop_thread() const {
        return thread_ID_ == current_thread::tid();
    }

    void assert_in_loop_thread() {
        if(!is_in_loop_thread()) {
            abort_not_in_loop_thread();
        }
    }

    bool event_handleing() const {
        return event_handleing_;
    }

    size_t queue_size() const {
        MutexLockGuard lock(mutex_);
        return pending_functors_.size();
    }

    void loop();
    void quit();
    // TODO
    // add poll return time
    void run_in_loop(Functor cb);
    void queue_in_loop(Functor cb);
    // TODO
    // add timer function
    void wakeup();
    // TODO
    // add channel function
    // add context function
    static EventLoop *get_event_loop_of_current_thread();

private:
    void abort_not_in_loop_thread();
    void handle_read();
    void do_pending_functors();
    // TODO
    // add print active channel for debug

    bool looping_;
    std::atomic<bool> quit_;
    bool event_handleing_;
    bool calling_pending_functors_;
    int64_t iteration_;
    const pid_t thread_ID_;
    // TODO
    // add timestamp poll return time
    // add unique_ptr of poller and timequeue
    int wakeup_fd_;
    // TODO
    // add context_
    // add channel
    mutable MutexLock mutex_;
    std::vector<Functor> pending_functors_;
};

} // namespace net

} // namespace web_server

#endif // WEB_SERVER_NET_EVENTLOOP_H