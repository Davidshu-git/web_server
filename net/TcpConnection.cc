/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/TcpConnection.h"

#include "base/Logging.h"

namespace web_server {

namespace net {

void default_connection_callback(const TcpConnectionPtr &conn) {
    LOG_TRACE << conn->local_addr().to_IP_port() << " -> "
              << conn->peer_addr().to_IP_port() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

void default_message_callback(const TcpConnectionPtr &,
                                               Buffer *buf,
                                               Timestamp) {
    buf->retrieve_all();
}

} // namespace net

} // namespace web_server