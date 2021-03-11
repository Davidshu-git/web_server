/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_EVENTLOOPTHREADPOOL_H
#define WEB_SERVER_NET_EVENTLOOPTHREADPOOL_H

#include <functional>
#include <string>
#include <vector>
#include <memory>

#include "base/Noncopyable.h"

namespace web_server {

namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : private Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThreadPool(EventLoop *base_loop, const std::string &name);
    ~EventLoopThreadPool();

    void set_thread_num(int num_threads) {
        num_threads_ = num_threads;
    }

    void start(const ThreadInitCallback &cb = ThreadInitCallback());
    EventLoop *get_next_loop();
    EventLoop *get_loop_from_hash(size_t hash_code);
    std::vector<EventLoop *> get_all_loops();

    bool is_started() const {
        return started_;
    }

    const std::string &name() const {
        return name_;
    }

private:
    EventLoop *base_loop_;
    std::string name_;
    bool started_;
    int num_threads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};

} // namespace net

} // namespace web_server

#endif // WEB_SERVER_NET_EVENTLOOPTHREADPOOL_H