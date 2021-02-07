/**
 * @brief source file of poll
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_POLLER_H
#define WEB_SERVER_NET_POLLER_H

#include <vector>
#include <map>

#include "base/Noncopyable.h"
#include "net/EventLoop.h"
#include "base/Timestamp.h"

namespace web_server {

namespace net {

class Channel;

class Poller : private Noncopyable {
public:
    using ChannelLists = std::vector<Channel *>;
    Poller(EventLoop *loop);
    virtual ~Poller();

    virtual Timestamp poll(int timeout_ms, ChannelLists *active_channels) = 0;

    virtual void update_channel(Channel *channel) = 0;
    virtual void remove_channel(Channel *channel) = 0;
    virtual bool has_channel(Channel *channel) const;

    void assert_in_loop_thread() const {
        owner_loop_->assert_in_loop_thread();
    }

    static Poller *new_default_poller(EventLoop *loop);
protected:
    using ChannelMap = std::map<int, Channel *>;
    ChannelMap channels_;
private:
    EventLoop *owner_loop_;
};

} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_POLL_H