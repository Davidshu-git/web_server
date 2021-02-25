/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/poller/PollPoller.h"

#include <poll.h>

#include <cerrno>
#include <cassert>

#include "base/Logging.h"
#include "net/Channel.h"

namespace web_server {

namespace net {

PollPoller::PollPoller(EventLoop *loop) : Poller(loop) {}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::poll(int timeout_ms, ChannelLists *active_channels) {
    int num_events = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout_ms);
    int saved_errno = errno;
    Timestamp now(Timestamp::now());
    if(num_events > 0) {
        LOG_TRACE << num_events << " events happened";
        fill_active_channels(num_events, active_channels);
    } else if (num_events == 0) {
        LOG_TRACE << " nothing happened";
    } else {
        if (saved_errno != EINTR) {
            errno = saved_errno;
            LOG_SYSERR << "PollPoller::poll()";
        }
    }
    return now;
}

void PollPoller::fill_active_channels(int num_events, ChannelLists *active_channels) const {
    for (auto pfd = pollfds_.cbegin(); pfd != pollfds_.cend() && num_events > 0; ++pfd) {
        if (pfd->revents > 0) {
            --num_events;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.cend());
            Channel *channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            active_channels->push_back(channel);
        }
    }
}

void PollPoller::update_channel(Channel *channel) {
    Poller::assert_in_loop_thread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    if (channel->index() < 0) {
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    } else {
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(idx > 0 && idx < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->is_nonevent()) {
            pfd.fd = -channel->fd()-1;
        }
    }
}

void PollPoller::remove_channel(Channel *channel) {
    Poller::assert_in_loop_thread();
    LOG_TRACE << "fd = " << channel->fd();
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->is_nonevent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd& pfd = pollfds_[idx];
    assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1);
    if (idx == (static_cast<int>(pollfds_.size()) - 1)) {
        pollfds_.pop_back();
    } else {
        int channel_at_end = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channel_at_end < 0) {
            channel_at_end = -channel_at_end - 1;
        }
        channels_[channel_at_end]->set_index(idx);
        pollfds_.pop_back();
    }
}

} // namespace net

} // namespace web_server