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

#include "base/Timestamp.h"

namespace web_server {

namespace net {

class EventLoop;

class Channel : Noncopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    void handle_event(Timestamp receive_time);
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

    void tie(const std::shared_ptr<void> &);
    int fd() const {
        return fd_;
    }
    int events() const {
        return events_;
    }
    // TODO set_revents
    bool is_nonevent() const {
        return events_ == k_nonevent;
    }
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

    bool is_reading() const {
        return events_ & k_readevent;
    }
    bool is_writing() const {
        return events_ & k_writeevent;
    }

    int index() {
        return index_;
    }

    void set_index(int idx) {
        index_ = idx;
    }
    // TODO Debug function
    EventLoop *owner_loop() {
        return loop_;
    }
    void remove();
private:
    void update();

    ReadEventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;

    static const int k_nonevent;
    static const int k_readevent;
    static const int k_writeevent;
    
    EventLoop *loop_;
    const int fd_;
    int events_;
    int index_;

    bool tied_;
    bool event_handling_;
    bool added_to_loop_;
};

} // namespace net

} // namespace web_server


#endif // WEB_SERVER_NET_CHANNEL_H