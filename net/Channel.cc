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
#include <sstream>
#include <memory>
#include <string>

#include "net/EventLoop.h"
#include "base/Logging.h"

namespace web_server {

namespace net {

const int Channel::k_nonevent = 0;
const int Channel::k_readevent = POLLIN | POLLPRI;
const int Channel::k_writeevent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      log_hup_(true),
      tied_(false),
      event_handling_(false),
      added_to_loop_(false) {
}

Channel::~Channel() {
    assert(!event_handling_);
    assert(!added_to_loop_);
    if (loop_->is_in_loop_thread()) {
        assert(!loop_->has_channel(this));
    }
}

void Channel::tie(const std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::update() {
    added_to_loop_ = true;
    loop_->update_channel(this);
}

void Channel::remove() {
    assert(is_nonevent());
    added_to_loop_ = false;
    loop_->remove_channel(this);
}

void Channel::handle_event(Timestamp receive_time) {
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        if (guard) {
            handle_event_with_guard(receive_time);
        }
    } else {
        handle_event_with_guard(receive_time);
    }
}

/**
 * @brief 实际执行io事件回调函数分发的函数
 * 根据revent的值可以获知目前触发了什么事件，那么一次对这些事件进行处理即可
 * @param receive_time 
 */
void Channel::handle_event_with_guard(Timestamp receive_time) {
    event_handling_ = true;
    // LOG_TRACE << revents_to_string();
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        if (log_hup_) {
            // LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
        }
        if (close_callback_) {
            close_callback_();
        }
    }
    if (revents_ & POLLNVAL) {
        // LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
    }
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (error_callback_) {
            error_callback_();
        }
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (read_callback_) {
            read_callback_(receive_time);
        }
    }
    if (revents_ & POLLOUT) {
        if (write_callback_) {
            write_callback_();
        }
    }
    event_handling_ = false;
}

std::string Channel::events_to_string() const {
    return events_to_string(fd_, events_);
}

std::string Channel::revents_to_string() const {
    return events_to_string(fd_, revents_);
}

std::string Channel::events_to_string(int fd, int ev) {
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & POLLIN)
        oss << "IN ";
    if (ev & POLLPRI)
        oss << "PRI ";
    if (ev & POLLOUT)
        oss << "OUT ";
    if (ev & POLLHUP)
        oss << "HUP ";
    if (ev & POLLRDHUP)
        oss << "RDHUP ";
    if (ev & POLLERR)
        oss << "ERR ";
    if (ev & POLLNVAL)
        oss << "NVAL ";
    return oss.str();
}

} // namespace net

} // namespace web_server