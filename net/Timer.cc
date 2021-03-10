/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/Timer.h"

namespace web_server {

namespace net {

AtomicInt64 Timer::s_num_created_;

void Timer::restart(Timestamp now) {
    if (repeat_) {
        expiration_ = add_time(now, interval_);
    } else {
        expiration_ = Timestamp::invalid();
    }
}

} // namespace net

} // namespace web_server