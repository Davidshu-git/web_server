/**
 * @brief internal headfile, not include this
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_POLLER_POLLPOLLER_H
#define WEB_SERVER_NET_POLLER_POLLPOLLER_H

#include "net/Poller.h"

#include <vector>

struct pollfd;

namespace web_server {

namespace net {

/**
 * @brief poller父类的子类实现
 * 底层使用的时poll系统函数
 */
class PollPoller : public Poller {
public:
    PollPoller(EventLoop *loop);
    ~PollPoller() override;

    Timestamp poll(int timeout_ms, ChannelLists *active_channels) override;
    void update_channel(Channel *channel) override;
    void remove_channel(Channel *channel) override;

private:
    void fill_active_channels(int num_events, ChannelLists *active_channels) const;
    using PollFdList = std::vector<struct pollfd>;
    PollFdList pollfds_;
};

} // namespace net

} // namespace web_server



#endif // WEB_SERVER_NET_POLLER_POLLPOLLER_H