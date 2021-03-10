/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_TIMER_H
#define WEB_SERVER_NET_TIMER_H

#include "base/Noncopyable.h"
#include "base/Timestamp.h"
#include "base/Atomic.h"
#include "net/Callbacks.h"

namespace web_server {

namespace net {

class Timer : private Noncopyable {
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(cb),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0),
          sequence_(s_num_created_.increment_get()) {}
          
    void run() const {
        callback_();
    }

    Timestamp expiration() const {
        return expiration_;
    }
    bool repeat() const {
        return repeat_;
    }
    int64_t sequence() const {
        return sequence_;
    }
    void restart(Timestamp now);

    static int64_t num_created() {
        return s_num_created_.get();
    }
private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static AtomicInt64 s_num_created_;
};

} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_TIMER_H