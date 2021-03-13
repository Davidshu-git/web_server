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
#include "base/Timestamp.h"
#include "net/TimerID.h"
#include "net/Callbacks.h"

namespace web_server {

namespace net {

class Channel;
class Poller;
class TimerQueue;

/**
 * @brief 创建了EventLoop对象的线程就是IO线程
 * 
 */
class EventLoop : private Noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp poll_return_time() const {
        return poll_return_time_;
    }

    int64_t iteration() const {
        return iteration_;
    }

    /**
     * @brief 判断当前执行线程是否是event_loop对象线程
     * 
     * @return true 
     * @return false 
     */
    bool is_in_loop_thread() const {
        return thread_ID_ == current_thread::tid();
    }

    /**
     * @brief 断言目前执行线程就是EventLoop对象线程
     */
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

    void run_in_loop(Functor cb);
    void queue_in_loop(Functor cb);
    
    TimerID run_at(Timestamp time, TimerCallback cb);
    TimerID run_after(double delay, TimerCallback cb);
    TimerID run_every(double interval, TimerCallback cb);
    void cancel(TimerID timer_ID);

    void wakeup();
    void update_channel(Channel *channel);
    void remove_channel(Channel *channel);
    bool has_channel(Channel *channel);
    // TODO
    // add context function

    /**
     * @brief Get the event loop of current thread object
     * 使用__thread 存储设施存放EventLoop类型指针，指向当前线程中的event_loop
     * @return EventLoop* 
     */
    static EventLoop *get_event_loop_of_current_thread();

private:
    using ChannelLists = std::vector<Channel *>;
    void abort_not_in_loop_thread();
    void handle_read();
    void do_pending_functors();
    
    void print_active_channels() const;

    // show status
    bool looping_;
    std::atomic<bool> quit_;
    bool event_handling_;
    bool calling_pending_functors_;

    int64_t iteration_;
    const pid_t thread_ID_;
    Timestamp poll_return_time_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timer_queue_;
    int wakeup_fd_;
    std::unique_ptr<Channel> wakeup_channel_;
    //add boost any

    // manage channel
    ChannelLists active_channels_;
    Channel *current_active_channel_;

    mutable MutexLock mutex_;
    std::vector<Functor> pending_functors_;
};

} // namespace net

} // namespace web_server

#endif // WEB_SERVER_NET_EVENTLOOP_H