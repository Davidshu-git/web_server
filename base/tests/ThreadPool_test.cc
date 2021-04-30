/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/ThreadPool.h"
#include "base/Logging.h"

#include <unistd.h>

#include <cstdio>
#include <functional>

void print() {
    printf("tid=%d\n", web_server::current_thread::tid());
}

void printString(const std::string& str) {
  // LOG_INFO << str;
  usleep(100*1000);
}

void test(int maxSize) {
    // LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
    web_server::ThreadPool pool("MainThreadPool");
    pool.set_max_queue_size(maxSize);
    pool.start(5);

    // LOG_WARN << "Adding";
    pool.run(print);
    pool.run(print);
    for (int i = 0; i < 100; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d", i);
        pool.run(std::bind(printString, std::string(buf)));
    }
    // LOG_WARN << "Done";

    web_server::CountDownLatch latch(1);
    pool.run(std::bind(&web_server::CountDownLatch::count_down, &latch));
    latch.wait();
    pool.stop();
}

int main() {
    test(0);
    test(1);
    test(5);
    test(10);
    test(50);
}