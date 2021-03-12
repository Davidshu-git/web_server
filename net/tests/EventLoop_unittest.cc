/**
 * @brief test file for eventloop
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/EventLoop.h"

#include <unistd.h>
#include <cassert>

#include "base/Thread.h"

using web_server::net::EventLoop;
using web_server::Thread;

void callback() {
  printf("callback(): pid = %d, tid = %d\n", getpid(), web_server::current_thread::tid());
//   EventLoop anotherLoop;
}

void thread_func() {
    printf("thread_func() pid = %d, tid = %d\n", getpid(), web_server::current_thread::tid());
    assert(EventLoop::get_event_loop_of_current_thread() == NULL);
    EventLoop loop;
    assert(EventLoop::get_event_loop_of_current_thread() == &loop);
    loop.run_after(1.0, callback);
    loop.loop();
}

int main() {
    printf("main() pid = %d, tid = %d\n", getpid(), web_server::current_thread::tid());
    assert(EventLoop::get_event_loop_of_current_thread() == NULL);
    EventLoop loop;
    assert(EventLoop::get_event_loop_of_current_thread() == &loop);

    Thread thread(thread_func);
    thread.start();
    loop.loop();
}