/**
 * @brief gcc buildin atomic function, template class for it
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_ATOMIC_H
#define WEB_SERVER_ATOMIC_H

#include <cstdint>
#include "base/Noncopyable.h"

namespace web_server{

namespace detail {

template <typename T>
class AtomicInteger : private Noncopyable {
public:
    AtomicInteger() : value_(0){}
    T get() {
        return __atomic_load_n(&value_, __ATOMIC_SEQ_CST);
    }

    T get_add(T x) {
        return __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST);
    }

    T add_get(T x) {
        return __atomic_add_fetch(&value_, x, __ATOMIC_SEQ_CST);
    }

    T get_set(T new_value) {
        return __atomic_exchange_n(&value_, new_value, __ATOMIC_SEQ_CST);
    }

    T increment_get() {
        return add_get(1);
    }

    T decrement_get() {
        return add_get(-1);
    }

    void add(T x) {
        get_add(x);
    }

    void increment() {
        increment_get();
    }

    void decrement() {
        decrement_get();
    }

private:
    volatile T value_;

};

} // namespace detail

using AtomicInt32 = detail::AtomicInteger<int32_t>;
using AtomicInt64 = detail::AtomicInteger<int64_t>;

} // namespace web_server

#endif // WEB_SERVER_ATOMIC_H