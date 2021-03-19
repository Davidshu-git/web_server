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

const int k_new = -1;       // 表示在epoll树上的状态
const int k_added = 1;      // 用于channel的index属性
const int k_deleted = 2;    // 让channel自己知道自身在epoll树上的状态

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
    int num_events = ::epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeout_ms);
    int saved_errno = errno;
    Timestamp now(Timestamp::now());
    if (num_events > 0) {
        LOG_TRACE << num_events << " events happened";
        fill_active_channels(num_events, active_channels);
        // 扩充events数组大小
        if (num_events == static_cast<int>(events_.size())) {
            events_.resize(events_.size() * 2);
        }
    } else if (num_events == 0) {
        LOG_TRACE << "nothing happened";
    } else {
        // 被系统中断了，忽略这种类型的错误
        if (saved_errno != EINTR) {
            errno = saved_errno;
            LOG_SYSERR << "EPollPoller::poll()";
        }
    }
    return now;
}

/**
 * @brief 从events数组中得到信息：文件描述符、感兴趣事件类型
 * 将得到的这些内容的channel放入active_channel之中
 * @param num_events 
 * @param active_channels 
 */
void EPollPoller::fill_active_channels(int num_events, ChannelLists *active_channels) const {
    assert(num_events <= static_cast<int>(events_.size()));
    for (int i = 0; i < num_events; ++i) {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        int fd = channel->fd();
        auto it = channels_.find(fd);
        assert(it != channels_.cend());
        assert(it->second == channel);
        channel->set_revents(events_[i].events);
        active_channels->push_back(channel);
    }
}

/**
 * @brief 必须在loop线程中执行
 * 
 * @param channel 
 */
void EPollPoller::update_channel(Channel *channel) {
    Poller::assert_in_loop_thread();
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd() 
              << " events = " << channel->events() 
              << " index = " << index;
    if (index == k_new || index == k_deleted) {
        int fd = channel->fd();
        // 若为新的channel，则channel map中是找不到这个channel的fd的
        // 直接添加这个channel
        if (index == k_new) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        // 若为已经删除的channel，channel map中可找到
        // 断言找到了这个channel
        } else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        // 将channel的index状态设置为added
        channel->set_index(k_added);
        // 在epoll树上注册这个channel
        update(EPOLL_CTL_ADD, channel);
    } else {
        // 若为已经添加过的channel
        // 则在epoll树上执行修改
        int fd = channel->fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == k_added);
        // 若该channel关注的事件为空了
        // 则需要在epoll树上删除改channle管理的事件
        if (channel->is_nonevent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(k_deleted);
        } else {
            // 若不是为空，则产生了一定变化，在epoll树上更新这个channel
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

/**
 * @brief 必须在loop线程中进行调用
 * 
 * @param channel 
 */
void EPollPoller::remove_channel(Channel *channel) {
    Poller::assert_in_loop_thread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    // 满足以下断言的才能进行channel删除
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->is_nonevent());
    int index = channel->index();
    assert(index == k_added || index == k_deleted);
    // 从channel map上删掉这个channel
    size_t n = channels_.erase(fd);
    assert(n == 1);
    if (index == k_added) {
        // 还要在epoll树上删掉这个channel
        update(EPOLL_CTL_DEL, channel);
    }
    // 表示这个channel在epoll树上是不存在的
    channel->set_index(k_new);
}

void EPollPoller::update(int operation, Channel *channel) {
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    // 关注的事件
    event.events = channel->events();
    // events数组中的data部分
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << operation_to_string(operation)
              << " fd = " << fd << " event = { " << channel->events_to_string() << " }";
    // 根据指定的operation，在epoll树上的对应fd上增、删、改event
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