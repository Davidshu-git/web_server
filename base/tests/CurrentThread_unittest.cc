/**
 * @brief current thread test
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/CurrentThread.h"

#include <assert.h>

#include <stdio.h>
#include <unistd.h>

int main() {
    using namespace web_server::current_thread;

    printf("tid_cached_tid = %d, func = %d\n", t_cached_tid, tid());
    printf("tid_string = %s, func = %s\n", t_tid_string, tid_string());
    printf("tid_string_length = %d, func = %d\n", t_tid_string_length, tid_string_length());
    printf("pid = %d\n", ::getpid());
    printf("is_main_thread = %d\n", is_main_thread());
    assert(is_main_thread());
}