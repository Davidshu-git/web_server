/**
 * @brief thread test file
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/Thread.h"
#include "base/CurrentThread.h"

#include "unistd.h"

void thread_func_1() {
    printf("tid = %d\n", web_server::current_thread::tid());
}

int main() {
    printf("pid = %d, tid = %d\n", ::getpid(), web_server::current_thread::tid());

    web_server::Thread thread_1(thread_func_1);
    thread_1.start();
    printf("thread_1.tid = %d\n", thread_1.tid());
    thread_1.join();
}