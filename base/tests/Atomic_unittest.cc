/**
 * @brief atomic_unittest file
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/Atomic.h"

#include <cassert>

int main() {
    {
        web_server::AtomicInt64 a0;
        assert(a0.get() == 0);
        assert(a0.get_add(1) == 0);
        assert(a0.get() == 1);
        assert(a0.add_get(2) == 3);
        assert(a0.get() == 3);
        assert(a0.increment_get() == 4);
        assert(a0.get() == 4);
        a0.increment();
        assert(a0.get() == 5);
        assert(a0.add_get(-3) == 2);
        assert(a0.get_set(100) == 2);
        assert(a0.get() == 100);
    }

    {
        web_server::AtomicInt32 a1;
        assert(a1.get() == 0);
        assert(a1.get_add(1) == 0);
        assert(a1.get() == 1);
        assert(a1.add_get(2) == 3);
        assert(a1.get() == 3);
        assert(a1.increment_get() == 4);
        assert(a1.get() == 4);
        a1.increment();
        assert(a1.get() == 5);
        assert(a1.add_get(-3) == 2);
        assert(a1.get_set(100) == 2);
        assert(a1.get() == 100);
    }

}