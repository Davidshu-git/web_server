/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/poller/EPollPoller.h"

#include <sys/epoll.h>
#include <poll.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstring>

#include "base/Logging.h"
#include "net/Channel.h"

namespace  {

const int k_new = -1;
const int k_added = 1;
const int k_deleted = 2;

} // namespace 

namespace web_server {

namespace net {

static_assert(EPOLLIN == POLLIN,        "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI,      "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT,      "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR,      "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP,      "epoll uses same flag values as poll");

EPollPoller::EPollPoller(EventLoop *loop) 
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(k_init_event_list_size) {
    if (epollfd_ < 0) {
        LOG_SYSFATAL << "EPollPoller::EPollPoller";
    }
}

EPollPoller::~EPollPoller() {
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeout_ms, ChannelLists *active_channels) {
    LOG_TRACE << "fd total count " << channels_.size();
    int num_events = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeout_ms);
    int saved_errno = errno;
    Timestamp now(Timestamp::now());
    if (num_events > 0) {
        LOG_TRACE << num_events << " events happened";
        fill_active_channels(num_events, active_channels);
        if (num_events == static_cast<int>(events_.size())) {
            events_.resize(events_.size() * 2);
        }
    } else if (num_events == 0) {
        LOG_TRACE << "nothing happened";
    } else {
        if (saved_errno != EINTR) {
            errno = saved_errno;
            LOG_SYSERR << "EPollPoller::poll()";
        }
    }
    return now;
}

void EPollPoller::fill_active_channels(int num_events, ChannelLists *active_channels) const {
    assert(num_events <= static_cast<int>(events_.size()));
    for (int i = 0; i < num_events; ++i) {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);

        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.cend());
        assert(it->second == channel);

        channel->set_revents(events_[i].events);
        active_channels->push_back(channel);
    }
}

void EPollPoller::update_channel(Channel *channel) {
    Poller::assert_in_loop_thread();
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd() 
              << " events = " << channel->events() 
              << " index = " << index;
    if (index == k_new || index == k_deleted) {
        int fd = channel->fd();
        if (index == k_new) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->set_index(k_added);
        update(EPOLL_CTL_ADD, channel);
    } else {
        int fd = channel->fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == k_added);
        if (channel->is_nonevent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(k_deleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::remove_channel(Channel *channel) {
    Poller::assert_in_loop_thread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->is_nonevent());
    int index = channel->index();
    assert(index == k_added || index == k_deleted);
    size_t n = channels_.erase(fd);
    assert(n == 1);
    if (index == k_added) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(k_new);
}

void EPollPoller::update(int operation, Channel *channel) {
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << operation_to_string(operation)
              << " fd = " << fd << " event = { " << channel->events_to_string() << " }";
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_SYSERR << "epoll_ctl op =" << operation_to_string(operation) << " fd =" << fd;
        } else {
            LOG_SYSFATAL << "epoll_ctl op =" << operation_to_string(operation) << " fd =" << fd;
        }
    }
}

const char* EPollPoller::operation_to_string(int op) {
    switch (op) {
        case EPOLL_CTL_ADD:
        return "ADD";
        case EPOLL_CTL_DEL:
        return "DEL";
        case EPOLL_CTL_MOD:
        return "MOD";
        default:
        assert(false && "ERROR op");
        return "Unknown Operation";
    }
}

} // namespace net

} // namespace web_server