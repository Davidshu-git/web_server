/**
 * @brief thread类的源文件
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/Thread.h"

#include <cstdio>
#include <cassert>
#include <string>
#include <pthread.h>
#include <sys/prctl.h>

#include "base/CurrentThread.h"

namespace web_server {

namespace detail {

/**
 * @brief 线程start时需要对其进行参数传递，这个类是对参数的封装
 * 在这个参数封装类中包含了需要运行的函数
 */
struct ThreadData {
    using ThreadFunction = web_server::Thread::ThreadFunction;
    ThreadFunction func_;
    std::string name_;
    pid_t *tid_;
    CountDownLatch *latch_;

    ThreadData(const ThreadFunction &func, const std::string &name, pid_t *tid, CountDownLatch *latch)
        : func_(func), name_(name), tid_(tid), latch_(latch) {}

    void run_in_thread() {
        *tid_ = web_server::current_thread::tid();
        tid_ = nullptr;
        latch_->count_down();
        latch_ = nullptr;

        web_server::current_thread::t_thread_name = name_.empty() ? "web_server_thread" : name_.c_str();
        // 将这个名称设置到系统进程名称中去
        ::prctl(PR_SET_NAME, web_server::current_thread::t_thread_name);

        func_();
        // 执行完线程该完成的任务后，这个线程的名称就改为finished
        web_server::current_thread::t_thread_name = "finished";
    }
};

/**
 * @brief start线程对象时，该函数作为线程的执行函数
 * 实际上是调用了ThreadData中的run_in_thread函数
 * 该函数实际上是对ThreadDate做了类型转换和资源回收
 * @param object 
 * @return void* 
 */
void *start_thread(void *object) {
    ThreadData *data = static_cast<ThreadData *>(object);
    data->run_in_thread();
    delete data;
    return nullptr;
}

} // namespace detail

/**
 * @brief Construct a new Thread
 * 但是并没有真正启动一个线程，真正的线程还没出现
 * 主要是为启动线程准备数据，如线程执行函数、线程名称等
 * @param func 
 * @param name 
 */
Thread::Thread(const ThreadFunction &func, const std::string &name)
    : started_(false), 
      joined_(false), 
      pthread_ID_(0), 
      tid_(0), 
      func_(func), 
      name_(name), 
      latch_(1) {
    set_default_name();
}

/**
 * @brief 使用原子方式计数，用于线程名称
 * 静态成员变量在类外定义初始化，调用默认构造函数初始化为0
 */
AtomicInt32 Thread::num_created_;

void Thread::set_default_name() {
    int num = num_created_.increment_get();
    if(name_.empty()) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Thread%d", num);
        name_ = buffer;
    }
}

/**
 * @brief Destroy the Thread:: Thread object
 * 若没有通过join回收，则使用detach的方式回收资源
 */
Thread::~Thread() {
    if(started_ && !joined_) {
        pthread_detach(pthread_ID_);
    }
}

/**
 * @brief 线程对象启动，创建运行的线程
 */
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

/**
 * @brief 线程对象主动销毁
 * @return int 
 */
int Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthread_ID_, NULL);
}

} // namespace web_server