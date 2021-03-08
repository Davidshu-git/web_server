/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_CALLBACKS_H
#define WEB_SERVER_CALLBACKS_H

#include <functional>
#include <memory>

#include "base/Timestamp.h"

namespace web_server {

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

template<typename T>
inline T *get_pointer(const std::shared_ptr<T> &ptr) {
    return ptr.get();
}

template<typename T>
inline T *get_pointer(const std::unique_ptr<T> &ptr) {
    return ptr.get();
}

namespace net {

class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &,
                                           Buffer*,
                                           Timestamp)>;
void default_connection_callback(const TcpConnectionPtr &conn);
void default_message_callback(const TcpConnectionPtr &conn,
                              Buffer *buffer,
                              Timestamp receive_time);
} // namespace net

} // namespace web_server


#endif // WEB_SERVER_CALLBACKS_H