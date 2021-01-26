/**
 * @brief This is public header file, for Noncopyable
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_BASE_NONCOPYABLE_H
#define WEB_SERVER_BASE_NONCOPYABLE_H

namespace web_server {

/**
 * @brief 不可拷贝的父类
 * 
 */
class Noncopyable {
public:
    // 将拷贝构造和拷贝赋值进行delete定义
    Noncopyable(const Noncopyable &) = delete;
    Noncopyable operator=(const Noncopyable &) = delete;
protected:
    // 定义默认的构造和析构函数为protected表示：不让该类用户构造和析构，允许该类派生类构造和析构
    Noncopyable() = default;
    ~Noncopyable() = default;
};

} // namespace web_server

#endif // WEB_SERVER_BASE_NONCOPYABLE_H