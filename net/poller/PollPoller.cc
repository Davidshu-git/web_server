/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/poller/PollPoller.h"

#include "poll.h"
#include "errno.h"

namespace web_server {

namespace net {

PollPoller::PollPoller(EventLoop *loop) : Poller(loop) {}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::poll(int timeout_ms, ChannelLists *active_channels) {
    int num_events = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout_ms);
    int saved_errno = errno;
    Timestamp now(Timestamp::now());
    if(num_events > 0) {
        
    }
}

} // namespace net

} // namespace web_server