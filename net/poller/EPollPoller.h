/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_POLLER_EPOLLPOLLER_H
#define WEB_SERVER_NET_POLLER_EPOLLPOLLER_H

#include "net/Poller.h"

#include <vector>

struct epoll_event;

namespace web_server {

namespace net {

class EPollPoller : public Poller {
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    Timestamp poll(int timeout_ms, ChannelLists *active_channels) override;
    void update_channel(Channel *channel) override;
    void remove_channel(Channel *channel) override;
private:
    static const int k_init_event_list_size = 16;

    static const char *operation_to_string(int op);

    void fill_active_channels(int num_events, ChannelLists *active_channels) const;
    void update(int operation, Channel *channel);

    using EventList = std::vector<struct epoll_event>;
    int epollfd_;
    EventList events_;
};

} // namespace net

} // namespace web_server

#endif // WEB_SERVER_NET_POLLER_EPOLLPOLLER_H