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
 * 将构造和析构函数声明权限为protected，该类本身不可构造析构
 * 派生类也可以构造和析构
 * 拷贝构造和拷贝赋值进行delete则子类不可拷贝
 * 将拷贝构造和拷贝赋值声明为private也可以达到相同效果
 */
class Noncopyable {
public:
    Noncopyable(const Noncopyable &) = delete;
    Noncopyable operator=(const Noncopyable &) = delete;
protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};

} // namespace web_server

#endif // WEB_SERVER_BASE_NONCOPYABLE_H