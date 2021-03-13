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

/**
 * @brief poller是抽象基类，使用必须有其子类实现
 * 定义了一些必须具备的借口，使用时使用该类指针即可调用子类方法
 * poller要更新channel中的revents信息，其必须要在IO线程中调用
 * 主要调用的函数时poll函数，传递给其eventloop指针告诉其所属的eventloop对象
 * 通过该eventloop对象调用assert_in_loop_thread即可判断是否在IO线程之中
 */
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
    /**
     * @brief 维护一个channelmap
     * 存放文件描述符和channel地址的映射信息
     */
    ChannelMap channels_;
private:
    EventLoop *owner_loop_;
};

} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_POLL_H