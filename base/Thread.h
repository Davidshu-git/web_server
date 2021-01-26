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
#include "base/Types.h"
#include "base/Atomic.h"

namespace web_server {
    /**
     * @brief 线程类
     * 
     */
class Thread : Noncopyable {
    // type alias of thread execute function
    using ThreadFunction = std::function<void()>;
public:
    /**
     * @brief Construct a new Thread object
     * 
     * @param func thread execute function
     * @param name thread name
     */
    explicit Thread(const ThreadFunction &func, const string &name = string());
    ~Thread();
    /**
     * @brief create thread
     * 
     */
    void start();

    /**
     * @brief thread create success
     * 
     * @return true 
     * @return false 
     */
    bool started() const;

    /**
     * @brief wrap pthread_join()
     * 
     * @return int 
     */
    int join();

    /**
     * @brief get thread id
     * 
     * @return pid_t 
     */
    pid_t tid() const;
    
    /**
     * @brief get thread name
     * 
     * @return const string& 
     */
    const string &name() const;



    
};

} // namespace web_server

#endif // WER_SERVER_BASE_THREAD_H