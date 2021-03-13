/**
 * @brief test file for eventloop
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/EventLoop.h"

#include <unistd.h>
#include <sys/timerfd.h>
#include <cassert>
#include <cstring>

#include "base/Thread.h"
#include "net/Channel.h"
#include "base/Timestamp.h"

using web_server::net::EventLoop;
using web_server::Thread;
using web_server::net::Channel;
using web_server::Timestamp;

void callback() {
  printf("callback(): pid = %d, tid = %d\n", getpid(), web_server::current_thread::tid());
//   EventLoop anotherLoop;
}

void thread_func() {
    printf("thread_func() pid = %d, tid = %d\n", getpid(), web_server::current_thread::tid());
    assert(EventLoop::get_event_loop_of_current_thread() == NULL);
    EventLoop loop;
    assert(EventLoop::get_event_loop_of_current_thread() == &loop);
    // loop.run_after(1.0, callback);
    loop.loop();
}

EventLoop *g_loop;

void time_out(Timestamp time) {
  printf("timeout!\n");
  g_loop->quit();
}

int main() {
    // printf("main() pid = %d, tid = %d\n", getpid(), web_server::current_thread::tid());
    // assert(EventLoop::get_event_loop_of_current_thread() == NULL);
    // EventLoop loop;
    // assert(EventLoop::get_event_loop_of_current_thread() == &loop);

    // Thread thread(thread_func);
    // thread.start();
    // loop.loop();
    // thread.join();
    EventLoop loop;
    g_loop = &loop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timerfd);
    channel.set_read_callback(time_out);
    channel.enable_reading();

    struct itimerspec how_long;
    memset(&how_long, 0, sizeof how_long);
    how_long.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &how_long, NULL);

    loop.loop();
    channel.disable_all();
    channel.remove();
    ::close(timerfd);
}