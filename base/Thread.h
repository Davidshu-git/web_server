/**
 * @brief 线程类文件
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WER_SERVER_BASE_THREAD_H
#define WER_SERVER_BASE_THREAD_H

#include <functional>
#include <pthread.h>
#include <string>

#include "base/Noncopyable.h"
#include "base/Atomic.h"
#include "base/CountDownLatch.h"

namespace web_server {

/**
 * @brief 使用线程类管理线程，保存线程信息
 */
class Thread : private Noncopyable {
public:
    using ThreadFunction = std::function<void()>;
    
    Thread(const ThreadFunction &func, const std::string &name = std::string());
    ~Thread();

    bool started() const {
        return started_;
    }

    pid_t tid() const {
        return tid_;
    }

    const std::string &name() const {
        return name_;
    }

    /**
     * @brief 获取已经创建了多少个线程类对象
     * @return int 
     */
    static int num_created() {
        return num_created_.get();
    }

    void start();

    int join();
    
private:
    bool started_;
    bool joined_;
    pthread_t pthread_ID_;
    pid_t tid_;
    ThreadFunction func_;
    std::string name_;
    CountDownLatch latch_;
    
    /**
     * @brief 保证获取该数时是一个原子操作
     * 保证线程安全，但是不需要锁机制消耗
     */
    static AtomicInt32 num_created_;
    
    void set_default_name();
};

} // namespace web_server

#endif // WER_SERVER_BASE_THREAD_H