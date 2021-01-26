/**
 * @brief Public header file, and it must only include public header file
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_EVENTLOOP_H
#define WEB_SERVER_NET_EVENTLOOP_H

#include <pthread.h>

#include "base/Noncopyable.h"

namespace web_server {

namespace net {

/**
 * @brief EventLoop的接口类，不要暴露过多细节
 * 
 */
class EventLoop : private Noncopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop();

    // 断言如果不是在线程中调用则退出
    void assertInLoopThread();

    // 调用线程是否为I/O线程
    // @return true/false
    bool isInLoopThread() const;

private:
    void abortNotInLoopThread();

    bool looping_;                      // 是否在事件循环中
    const pid_t threadID_;              // EventLoop所属线程ID
};

} // namespace net

} // namespace web_server

#endif // WEB_SERVER_NET_EVENTLOOP_H