/**
 * @brief copyable base class ,value type class
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_BASE_COPYABLE_H
#define WEB_SERVER_BASE_COPYABLE_H

namespace web_server {

/**
 * @brief 声明为protected权限
 * 对于派生类仍然是可用的，派生类拥有基类的构造和析构函数访问权限
 * 可正常进行copy操作，因为默认有拷贝构造和拷贝赋值
 */
class Copyable {
protected:
    Copyable() = default;
    ~Copyable() = default;
};

} // namespace web_server


#endif // WEB_SERVER_BASE_COPYABLE_H