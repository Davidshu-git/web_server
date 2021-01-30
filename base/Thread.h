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
#include <pthread.h>

#include "base/Noncopyable.h"
#include "base/Types.h"
#include "base/Atomic.h"
#include "base/CountDownLatch.h"

namespace web_server {

class Thread : private Noncopyable {
public:
    using ThreadFunction = std::function<void()>;
    Thread(const ThreadFunction &func, const string &name = string());
    ~Thread();

    bool started() const {
        return started_;
    }

    pid_t tid() const {
        return tid_;
    }

    const string &name() const {
        return name_;
    }

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
    string name_;
    CountDownLatch latch_;
    
    static AtomicInt32 num_created_;
    
    void set_default_name();
};

} // namespace web_server

#endif // WER_SERVER_BASE_THREAD_H