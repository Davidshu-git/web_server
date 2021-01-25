/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WER_SERVER_BASE_THREAD_H
#define WER_SERVER_BASE_THREAD_H

#include <functional>
#include <string>
#include <iostream>

#include "base/Noncopyable.h"

namespace web_server {
    using std::string;
    class Thread : Noncopyable {
        // 线程执行函数的类型
        using ThreadFunction = std::function<void()>;
        
    };

} // namespace web_server

#endif // WER_SERVER_BASE_THREAD_H