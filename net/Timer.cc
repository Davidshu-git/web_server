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

/**
 * @brief 定义重启timer时的操作
 * 若是重复，则叠加区间时间
 * 若是不重复，则直接将到期时间置为失效
 * @param now 
 */
void Timer::restart(Timestamp now) {
    if (repeat_) {
        expiration_ = add_time(now, interval_);
    } else {
        expiration_ = Timestamp::invalid();
    }
}

} // namespace net

} // namespace web_server