/**
 * @brief channel for I/O
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_CHANNEL_H
#define WEB_SERVER_NET_CHANNEL_H

#include "base/Noncopyable.h"

#include <functional>
#include <memory>
#include <string>

#include "base/Timestamp.h"

namespace web_server {

namespace net {

class EventLoop;

/**
 * @brief channel负责管理io事件触发后的回调函数
 * 只负责一个文件描述符的io事件分发，将不同的io事件分发给不同的回调函数
 * 每个channel对象只属于一个eventLoop线程
 * 其成员函数只能在IO线程进行调用，不用加锁
 */
class Channel : private Noncopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    void handle_event(Timestamp receive_time);
    void tie(const std::shared_ptr<void> &);

    void set_read_callback(ReadEventCallback cb) {
        read_callback_ = cb;
    }

    void set_write_callback(EventCallback cb) {
        write_callback_ = cb;
    }

    void set_close_callback(EventCallback cb) {
        close_callback_ = cb;
    }

    void set_error_callback(EventCallback cb) {
        error_callback_ = cb;
    }
    
    int fd() const {
        return fd_;
    }

    int events() const {
        return events_;
    }

    void set_revents(int revents) {
        revents_ = revents;
    }

    // 判断是不是对所有事件都不感兴趣了
    bool is_nonevent() const {
        return events_ == k_nonevent;
    }

    // 使能读事件，表示该channel管理的fd要对读事件感兴趣
    void enable_reading() {
        events_ |= k_readevent;
        update();
    }

    void disable_reading() {
        events_ &= ~k_readevent;
        update();
    }

    void enable_writing() {
        events_ |= k_writeevent;
        update();
    }

    void disable_writing() {
        events_ &= ~k_writeevent;
        update();
    }

    void disable_all() {
        events_ = k_nonevent;
        update();
    }

    // 表示是不是正在跟踪读事件
    bool is_reading() const {
        return events_ & k_readevent;
    }

    bool is_writing() const {
        return events_ & k_writeevent;
    }

    // 辅助记录
    int index() {
        return index_;
    }

    void set_index(int idx) {
        index_ = idx;
    }
    
    std::string events_to_string() const;
    std::string revents_to_string() const;

    void do_not_log_hup() {
        log_hup_ = false;
    }

    EventLoop *owner_loop() {
        return loop_;
    }

    void remove();

private:
    ReadEventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;

    static const int k_nonevent;
    static const int k_readevent;
    static const int k_writeevent;
    
    EventLoop *loop_;                   // 指向对应的loop对象
    const int fd_;                      // 其管理的fd
    int events_;                        // 用户关心的事件
    int revents_;                       // 目前触发事件
    int index_;
    bool log_hup_;

    std::weak_ptr<void> tie_;
    bool tied_;
    bool event_handling_;
    bool added_to_loop_;                // 表示该channel是否添加到loop对象中

    static std::string events_to_string(int fd, int ev);
    void update();
    void handle_event_with_guard(Timestamp receive_time);
};

} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_CHANNEL_H