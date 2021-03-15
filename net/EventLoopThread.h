/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_EVENTLOOPTHREAD_H
#define WEB_SERVER_NET_EVENTLOOPTHREAD_H

#include <string>

#include "base/Noncopyable.h"
#include "base/Thread.h"
#include "base/Mutex.h"
#include "base/Condition.h"

namespace web_server {

namespace net {

class EventLoop;

/**
 * @brief eventloop线程管理类
 * 为了便于创建多个IO线程
 */
class EventLoopThread : private Noncopyable {
public:
    using ThreadInitCallback = std::function<void (EventLoop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                    const std::string &name = std::string());
    ~EventLoopThread();
    EventLoop *start_loop();

private:
    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    ThreadInitCallback callback_;

    void thread_func();
};

} // namespace net

} // namespace web_server

#endif // WEB_SERVER_NET_EVENTLOOPTHREAD_H