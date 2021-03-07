/**
 * @brief unit test for buffer
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "net/Buffer.h"

#include <cassert>
#include <iostream>
#include <string>

using web_server::net::Buffer;
using std::cout;
using std::endl;
using std::string;

void test_buffer_append_retrieve() {
    Buffer buf;
    cout << "start test_append_retrieve" << endl;
    assert(buf.readable_bytes() == 0);
    assert(buf.writable_bytes() == Buffer::k_initial_size);
    assert(buf.prependable_bytes() == Buffer::k_cheap_prepend);

    const string str(200, 'x');
    buf.append(str.c_str(), str.size());
    assert(buf.readable_bytes() == str.size());
    assert(buf.writable_bytes() == Buffer::k_initial_size - str.size());
    assert(buf.prependable_bytes() == Buffer::k_cheap_prepend);

    const string str2 = buf.retrieve_as_string(50);
    assert(str2.size() == 50);
    assert(buf.readable_bytes() == str.size() - str2.size());
    assert(buf.writable_bytes() == Buffer::k_initial_size - str.size());
    assert(buf.prependable_bytes() == Buffer::k_cheap_prepend + str2.size());
    assert(str2 == string(50, 'x'));

    buf.append(str.c_str(), str.size());
    assert(buf.readable_bytes() == 2 * str.size() - str2.size());
    assert(buf.writable_bytes() == Buffer::k_initial_size - 2 * str.size());
    assert(buf.prependable_bytes() == Buffer::k_cheap_prepend + str2.size());

   const string str3 = buf.retrieve_all_as_string();
   assert(str3.size() == 350);
   assert(buf.readable_bytes() == 0);
   assert(buf.writable_bytes() == Buffer::k_initial_size);
   assert(buf.prependable_bytes() == Buffer::k_cheap_prepend);
   assert(str3 == string(350, 'x'));

    cout << "test finish successful" << endl;
}

void test_buffer_grow() {
    cout << "test buffer grow" << endl;
    Buffer buf;
    const string str(400, 'y');
    buf.append(str.c_str(), str.size());
    assert(buf.readable_bytes() == 400);
    assert(buf.writable_bytes() == Buffer::k_initial_size - 400);

    buf.retrieve(50);
    assert(buf.readable_bytes() == 350);
    assert(buf.writable_bytes() == Buffer::k_initial_size - 400);
    assert(buf.prependable_bytes() == Buffer::k_cheap_prepend + 50);

    const string str2(1000, 'z');
    buf.append(str2.c_str(), str2.size());
    assert(buf.readable_bytes() == 1350);
    assert(buf.writable_bytes() == 0);
    assert(buf.prependable_bytes() == Buffer::k_cheap_prepend + 50);

    buf.retrieve_all();
    assert(buf.readable_bytes() == 0);
    assert(buf.writable_bytes() == 1400);
    assert(buf.prependable_bytes() == Buffer::k_cheap_prepend);

    cout << "test finish successful" << endl;

}

int main() {
    test_buffer_append_retrieve();
    test_buffer_grow();
}
