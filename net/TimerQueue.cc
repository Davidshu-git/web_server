/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/TimerQueue.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <cassert>

#include "base/Logging.h"
#include "net/TimerID.h"
#include "net/Timer.h"
#include "net/EventLoop.h"


namespace web_server {

namespace net {

namespace detail {

int create_timerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return timerfd;
}

struct timespec how_much_time_from_now(Timestamp when) {
    int64_t microseconds = when.micro_seconds_since_epoch()
                           - Timestamp::now().micro_seconds_since_epoch();
    if (microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::k_micro_seconds_per_second);
    ts.tv_nsec = static_cast<long>(microseconds % Timestamp::k_micro_seconds_per_second * 1000);
    return ts;
}

/**
 * @brief 重设timerfd的触发时间
 * 
 * @param timerfd 
 * @param expiration 
 */
void reset_timerfd(int timerfd, Timestamp expiration) {
    struct itimerspec new_value;
    struct itimerspec old_value;
    memset(&new_value, 0, sizeof new_value);
    memset(&old_value, 0, sizeof old_value);
    new_value.it_value = how_much_time_from_now(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
    if (ret) {
        LOG_SYSERR << "timerfd_settime()";
    }
}

/**
 * @brief timerfd触发了一个读事件
 * 那么要将这个读数据取出，不然若是使用的poll则是level触发模式
 * 则会一直触发这个事件而不会计时
 * @param timerfd 
 * @param now 
 */
void read_timerfd(int timerfd, Timestamp now) {
    uint64_t how_many;
    ssize_t n = ::read(timerfd, &how_many, sizeof how_many);
    LOG_TRACE << "TimerQueue::handle_read() " << how_many << " at " << now.to_string();
    if (n != sizeof(how_many)) {
        LOG_ERROR << "TimerQueue::handle_read() reads " << n << " bytes instead of 8";
    }
}

} // namespace detail

TimerQueue::TimerQueue(EventLoop *loop) 
    : loop_(loop),
      timerfd_(detail::create_timerfd()),
      timerfd_channel_(loop, timerfd_),
      timers_(),
      calling_expired_timers_(false) {
    timerfd_channel_.set_read_callback(std::bind(&TimerQueue::handle_read, this));
    timerfd_channel_.enable_reading();
}

TimerQueue::~TimerQueue() {
    timerfd_channel_.disable_all();
    timerfd_channel_.remove();
    ::close(timerfd_);
    for (const Entry &timer : timers_) {
        delete timer.second;
    }
}

/**
 * @brief 为了实现在其他线程也可调用add_timer
 * 不使用锁机制的情况下，调用run_in_loop即可
 * add_timer_in_loop实际负责add动作
 * 本函数只负责将这个请求进行转发
 * @param cb 
 * @param when 
 * @param interval 
 * @return TimerID 序列值实际上就是创建的第多少个timer
 */
TimerID TimerQueue::add_timer(TimerCallback cb, Timestamp when, double interval) {
    Timer *timer = new Timer(cb, when, interval);
    loop_->run_in_loop(std::bind(&TimerQueue::add_timer_in_loop, this, timer));
    return TimerID(timer, timer->sequence());
}

/**
 * @brief 将一个timer进行取消操作
 * 该函数不需要运行在io线程，因为进行的是转发取消操作
 * 将操作转发给IO线程
 * @param timer_ID 
 */
void TimerQueue::cancel(TimerID timer_ID) {
    loop_->run_in_loop(std::bind(&TimerQueue::cancel_in_loop, this, timer_ID));
}

/**
 * @brief 该函数必须在IO线程中执行
 * 若是最早执行的一个timer，那么直接进行timerfd设置
 * @param timer 
 */
void TimerQueue::add_timer_in_loop(Timer *timer) {
    loop_->assert_in_loop_thread();
    bool earliest_changed = insert(timer);
    if (earliest_changed) {
        detail::reset_timerfd(timerfd_, timer->expiration());
    }
}

/**
 * @brief 真正执行cancel操作的函数
 * 
 * @param timer_ID 
 */
void TimerQueue::cancel_in_loop(TimerID timer_ID) {
    loop_->assert_in_loop_thread();
    assert(timers_.size() == active_timers_.size());
    ActiveTimer timer(timer_ID.timer_, timer_ID.sequence_);
    auto it = active_timers_.find(timer);
    if (it != active_timers_.end()) {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1);
        delete it->first;
        active_timers_.erase(it);
    } else if (calling_expired_timers_) {
        canceling_timers_.insert(timer);
    }
    assert(timers_.size() == active_timers_.size());
}

/**
 * @brief timerqueue自己的timerfd到期后执行的函数
 * 到期后就会触发读事件，涉及到对类成员变量的修改
 * 若不加锁，则需要在IO线程中执行
 * 在该函数中执行所有到期的定时器回调函数
 */
void TimerQueue::handle_read() {
    loop_->assert_in_loop_thread();
    Timestamp now(Timestamp::now());
    detail::read_timerfd(timerfd_, now);

    std::vector<Entry> expired = get_expired(now);
    calling_expired_timers_ = true;
    canceling_timers_.clear();

    for (const Entry &it : expired) {
        it.second->run();
    }
    calling_expired_timers_ = false;
    reset(expired, now);
}

/**
 * @brief 移除已经到期的timer，并以vector形式返回这些到期timer
 * @param now 
 * @return std::vector<TimerQueue::Entry> 
 */
std::vector<TimerQueue::Entry> TimerQueue::get_expired(Timestamp now) {
    assert(timers_.size() == active_timers_.size());
    std::vector<Entry> expired;
    // 由于第二个参数设置的是最大值，理论上找到的end是大于等于这个最大值的
    // 但是这个最大值是不可能取到的，所有相同now值的成员都会小于这个最大值
    // 所以找到的值一定满足*end >= sentry > now值对应成员
    // 则end->first > now
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (const Entry &it : expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = active_timers_.erase(timer);
        assert(n == 1);
    }
    
    assert(timers_.size() == active_timers_.size());
    return expired;
}

/**
 * @brief 若是存在可重复timer则进行重复设置
 * 若是该timer不可重复，那么直接进行资源释放
 * 若没有后续的timer了，则要设置timerqueue的timerfd为无效
 * @param expired 
 * @param now 
 */
void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
    Timestamp next_expire;
    for (const Entry &it :expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        if (it.second->repeat()
            && canceling_timers_.find(timer) == canceling_timers_.end()) {
                it.second->restart(now);
                insert(it.second);
        } else {
            delete it.second;
        }
    }

    if (!timers_.empty()) {
        next_expire = timers_.begin()->second->expiration();
    }

    if (next_expire.valid()) {
        detail::reset_timerfd(timerfd_, next_expire);
    }
}

/**
 * @brief 该函数必须在IO线程中执行
 * 记录这个timer是不是最早执行的一个
 * @param timer 
 * @return true 
 * @return false 
 */
bool TimerQueue::insert(Timer *timer) {
    loop_->assert_in_loop_thread();
    assert(timers_.size() == active_timers_.size());
    bool earliest_changed = false;
    Timestamp when = timer->expiration();
    auto it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliest_changed = true;
    }
    {
        std::pair<TimerList::iterator, bool> result
            = timers_.insert(Entry(when, timer));
        assert(result.second);
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result
            = active_timers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second);
    }
    assert(timers_.size() == active_timers_.size());
    return earliest_changed;
}

} // namespace net

} // namespace web_server