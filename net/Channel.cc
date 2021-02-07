/**
 * @brief source file for channel
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/Channel.h"

#include <poll.h>
#include <cassert>

#include "net/EventLoop.h"

namespace web_server {

namespace net {

const int Channel::k_nonevent = 0;
const int Channel::k_readevent = POLLIN | POLLPRI;
const int Channel::k_writeevent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      index_(-1),
      tied_(false),
      event_handling_(false),
      added_to_loop_(false) {
}

Channel::~Channel() {
    assert(!event_handling_);
    assert(!added_to_loop_);
    // if (loop_->is_in_loop_thread()) {
    //     assert(!loop_->)
    // }
}

} // namespace net

} // namespace web_server