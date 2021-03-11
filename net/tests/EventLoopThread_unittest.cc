/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */


#include "net/EventLoopThread.h"

#include <unistd.h>

#include "net/EventLoop.h"

using namespace web_server;
using namespace web_server::net;

void print(EventLoop *p = NULL) {
    printf("print: pid = %d, tid = %d, loop = %p\n",
           ::getpid(), current_thread::tid(), p);
}

void quit(EventLoop *p) {
    print(p);
    p->quit();
}

int main() {
    print();
    {
        EventLoopThread thr1;
    }
    {
        EventLoopThread thr2;
        EventLoop *loop = thr2.start_loop();
        loop->run_in_loop(std::bind(print, loop));
        current_thread::sleep_usec(500 * 1000);
    }
    {
        EventLoopThread thr3;
        EventLoop *loop = thr3.start_loop();
        loop->run_in_loop(std::bind(quit, loop));
        current_thread::sleep_usec(500 * 1000);
    }
}