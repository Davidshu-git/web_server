/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/Poller.h"
#include "net/poller/EPollPoller.h"
#include "net/poller/PollPoller.h"

namespace web_server {

namespace net {

Poller *Poller::new_default_poller(EventLoop *loop) {
    if(::getenv("WEB_SERVER_USE_POLL")) {
        return new PollPoller(loop);
    } else {
        return new EPollPoller(loop);
    }
}

} // namespace net

} // namespace web_server