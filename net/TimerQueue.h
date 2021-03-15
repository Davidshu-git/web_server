/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_TIMERQUEUE_H
#define WEB_SERVER_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include "base/Noncopyable.h"
#include "base/Timestamp.h"
#include "base/Mutex.h"
#include "net/Channel.h"
#include "net/Callbacks.h"

namespace web_server {

namespace net {

class EventLoop;
class Timer;
class TimerID;

/**
 * @brief 管理所有定时器
 */
class TimerQueue : private Noncopyable {
public:
    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    TimerID add_timer(TimerCallback cb, Timestamp when, double interval);
    void cancel(TimerID timer_ID);

private:
    using Entry = std::pair<Timestamp, Timer *>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer *, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    EventLoop *loop_;                                   // 指向所属的eventloop
    const int timerfd_;                                 // timerfd
    Channel timerfd_channel_;                           // timerfd对应的channel，管理其回调事件
    TimerList timers_;                                  // timer的序列底层使用set保存
    ActiveTimerSet active_timers_;                      // 活跃timer序列，底层也是set
    bool calling_expired_timers_;                       // 表示正在调用过期timer
    ActiveTimerSet canceling_timers_;                   // 表示已经过期的timer

    void add_timer_in_loop(Timer *timer);
    void cancel_in_loop(TimerID timer_ID);
    void handle_read();
    std::vector<Entry> get_expired(Timestamp now);
    void reset(const std::vector<Entry> &expired, Timestamp now);
    bool insert(Timer *timer);
};


} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_TIME_QUEUE_H