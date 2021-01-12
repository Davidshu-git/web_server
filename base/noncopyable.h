// header file for eventloop
// Copyright (C) 2021, David Shu. All rights reserved.
//
// Use of this source code is governed by a GPL license
// Author: David Shu(a294562476@gmail.com)

#ifndef WEB_SERVER_BASE_NONCOPYABLE_H
#define WEB_SERVER_BASE_NONCOPYABLE_H

namespace web_server {

    // 不可拷贝类的父类
    class noncopyable {
    public:
        // 将拷贝构造和拷贝赋值进行delete定义
        // 将此类定义为不可拷贝
        noncopyable(const noncopyable &) = delete;
        noncopyable operator=(const noncopyable &) = delete;
    protected:
        // 定义默认的构造和析构函数为protected表示：
        // 不让该类用户构造和析构
        // 允许该类派生类构造和析构
        noncopyable() = default;
        ~noncopyable() = default;
    };
} // namespace web_server

#endif