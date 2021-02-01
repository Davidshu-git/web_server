/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/Thread.h"

#include <cstdio>
#include <cassert>
#include <pthread.h>
#include <sys/prctl.h>


#include "base/Types.h"
#include "base/CurrentThread.h"

namespace web_server {

namespace detail {

void after_fork() {
    current_thread::t_cached_tid = 0;
    current_thread::t_thread_name = "main";
    current_thread::tid();
}

class ThreadNameInitializer {
public:
    ThreadNameInitializer() {
        current_thread::t_thread_name = "main";
        current_thread::tid();
        pthread_atfork(NULL, NULL, &after_fork);
    }
};

ThreadNameInitializer init;

struct ThreadData {
    using ThreadFunction = web_server::Thread::ThreadFunction;
    ThreadFunction func_;
    string name_;
    pid_t *tid_;
    CountDownLatch *latch_;

    ThreadData(const ThreadFunction &func, const string &name, pid_t *tid, CountDownLatch *latch)
        : func_(func), name_(name), tid_(tid), latch_(latch) {}

    void run_in_thread() {
        *tid_ = web_server::current_thread::tid();
        tid_ = nullptr;
        latch_->count_down();
        latch_ = nullptr;

        web_server::current_thread::t_thread_name = name_.empty() ? "web_server_thread" : name_.c_str();
        ::prctl(PR_SET_NAME, web_server::current_thread::t_thread_name);

        func_();
        web_server::current_thread::t_thread_name = "finished";
    }
};

void *start_thread(void *object) {
    ThreadData *data = static_cast<ThreadData *>(object);
    data->run_in_thread();
    delete data;
    return NULL;
}

} // namespace detail

Thread::Thread(const ThreadFunction &func, const string &name)
    : started_(false), 
      joined_(false), 
      pthread_ID_(0), 
      tid_(0), 
      func_(func), 
      name_(name), 
      latch_(1) {
    set_default_name();
}

Thread::~Thread() {
    if(started_ && !joined_) {
        pthread_detach(pthread_ID_);
    }
}

AtomicInt32 Thread::num_created_;

void Thread::set_default_name() {
    int num = num_created_.increment_get();
    if(name_.empty()) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Thread%d", num);
        name_ = buffer;
    }
}

void Thread::start() {
    assert(!started_);
    started_ = true;

    auto data = new detail::ThreadData(func_, name_, &tid_, &latch_);
    if(pthread_create(&pthread_ID_, NULL, &detail::start_thread, data)) {
        started_ = false;
        delete data;
    } else {
        latch_.wait();
        assert(tid_ > 0);
    }
}

int Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthread_ID_, NULL);
}

} // namespace web_server