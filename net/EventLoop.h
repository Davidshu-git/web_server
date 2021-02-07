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
#include <memory>

#include "base/Noncopyable.h"
#include "base/Mutex.h"
#include "base/CurrentThread.h"

namespace web_server {

namespace net {

class Channel;
class Poller;

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

    bool event_handling() const {
        return event_handling_;
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
    void update_channel(Channel *channel);
    void remove_channel(Channel *channel);
    bool has_channel(Channel *channel);
    // TODO
    // add context function
    static EventLoop *get_event_loop_of_current_thread();

private:
    using ChannelLists = std::vector<Channel *>;
    void abort_not_in_loop_thread();
    void handle_read();
    void do_pending_functors();
    // TODO
    // add print active channel for debug

    bool looping_;
    std::atomic<bool> quit_;
    bool event_handling_;
    bool calling_pending_functors_;
    int64_t iteration_;
    const pid_t thread_ID_;
    // TODO
    // add timestamp poll return time
    std::unique_ptr<Poller> poller_;
    // add unique_ptr of poller and timequeue
    int wakeup_fd_;
    // TODO
    // add context_
    ChannelLists active_channels_;
    Channel *current_active_channel_;
    mutable MutexLock mutex_;
    std::vector<Functor> pending_functors_;
};

} // namespace net

} // namespace web_server

#endif // WEB_SERVER_NET_EVENTLOOP_H