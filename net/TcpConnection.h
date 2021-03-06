/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_NET_TCPCONNECTION_H
#define WEB_SERVER_NET_TCPCONNECTION_H

#include <memory>
#include <string>

#include <boost/any.hpp>

#include "base/Noncopyable.h"
#include "base/Logging.h"
#include "net/Buffer.h"
#include "net/InetAddress.h"
#include "net/Callbacks.h"

struct tcp_info;

namespace web_server {

namespace net {

class Channel;
class EventLoop;
class Socket;

/**
 * @brief 表示一个已建立的tcp连接
 * 一个连接由这样的几部分组成：一对地址、名称、所属loop、sockfd
 * 该连接对象要负责管理回调：连接回调、消息回调、写完成回调、高水位回调、关闭回调
 */
class TcpConnection : private Noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *loop,
                  const std::string &name,
                  int sockfd,
                  const InetAddress &local_addr,
                  const InetAddress &peer_addr);
    ~TcpConnection();

    EventLoop *get_loop() const {
        return loop_;
    }
    const std::string &name() const {
        return name_;
    }
    const InetAddress &local_addr() const {
        return local_addr_;
    }
    const InetAddress &peer_addr() const {
        return peer_addr_;
    }
    bool connected() const {
        return state_ == kConnected;
    }
    bool disconnected() const {
        return state_ == kDisconnected;
    }
    const boost::any &get_context() const {
        return context_;
    }
    boost::any *get_mutable_context() {
        return &context_;
    }
    std::string get_tcp_info_string() const;
    
    void send(const void *message, size_t len);
    void send(const std::string &message);
    void shutdown();
    void connection_established();
    void connection_destroyed();
    // 设置禁用Nagle算法
    void set_tcp_no_delay(bool on);
    void set_context(const boost::any &context) {
        context_ = context;
    }
    void set_connection_callback(const ConnectionCallback &cb) {
        connection_callback_ = cb;
    }
    void set_message_callback(const MessageCallback &cb) {
        message_callback_ = cb;
    }
    void set_write_complete_callback(const WriteCompleteCallback &cb) {
        write_complete_callback_ = cb;
    }
    void set_high_water_mark_callback(const HighWaterMarkCallback &cb, size_t high_water_mark) {
        high_water_mark_callback_ = cb;
        high_water_mark_ = high_water_mark;
    }
    void set_close_callback(const CloseCallback &cb) {
        close_callback_ = cb;
    }
private:
    enum StateE {kDisconnected, kConnecting, kConnected, kDisconnecting};
    void handle_read(Timestamp receive_time);
    void handle_write();
    void handle_close();
    void handle_error();
    
    void send_in_loop(const std::string &message);
    void shutdown_in_loop();

    void set_state(StateE s) {
        state_ = s;
    }
    
    const std::string state_to_string() const;

    EventLoop *loop_;
    const std::string name_;
    StateE state_;                                      // 存储该连接状态
    std::unique_ptr<Socket> socket_;                    // 管理socket
    std::unique_ptr<Channel> channel_;                  // 需要使用Channel管理socket触发回调
    const InetAddress local_addr_;
    const InetAddress peer_addr_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    HighWaterMarkCallback high_water_mark_callback_;
    CloseCallback close_callback_;
    size_t high_water_mark_;
    Buffer input_buffer_;                               // 输入数据缓冲，负责接收数据
    Buffer output_buffer_;                              // 输出数据缓冲，负责发送数据
    boost::any context_;
};

} // namespace net

} // namespace web_server

#endif // WEB_SERVER_NET_TCPCONNECTION_H