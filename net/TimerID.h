/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_TIMERID_H
#define WEB_SERVER_NET_TIMERID_H

#include "base/Copyable.h"

#include <cstdint>

namespace web_server {

namespace net {

class Timer;

class TimerID : public Copyable {
    friend class TimerQueue;
public:
    TimerID() : timer_(nullptr), sequence_(0) {}
    TimerID(Timer *timer, int64_t seq) : timer_(timer), sequence_(seq) {}
private:
    Timer *timer_;
    int64_t sequence_;
};

} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_TIMERID_H