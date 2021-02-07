/**
 * @brief poller source file
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/Poller.h"
#include "net/Channel.h"

namespace web_server {

namespace net {

Poller::Poller(EventLoop *loop) : owner_loop_(loop) {}
Poller::~Poller() = default;

bool Poller::has_channel(Channel *channel) const {
    assert_in_loop_thread();
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && channel == it->second;
}

} // namespace net

} // namespace web_server