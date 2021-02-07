/**
 * @brief timestamp test
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/Timestamp.h"

#include <vector>

using web_server::Timestamp;
using web_server::time_difference;
using web_server::add_time;

void pass_by_value(Timestamp x) {
    printf("%s\n", x.to_string().c_str());
}

void pass_by_const_reference(Timestamp &x) {
    printf("%s\n", x.to_string().c_str());
}

void benchmark() {
    const int k_number = 1000 * 1000;
    std::vector<Timestamp> stamps;
    stamps.reserve(k_number);
    for(int i = 0; i < k_number; ++i) {
        stamps.push_back(Timestamp::now());
    }
    printf("%s\n", stamps.front().to_string().c_str());
    printf("%s\n", stamps.back().to_string().c_str());
    printf("%f\n", time_difference(stamps.back(), stamps.front()));
    printf("%s\n", add_time(stamps.back(), 1.1).to_string().c_str());
    int increment[100] = {0};
    int64_t start = stamps.front().micro_seconds_since_epoch();
    for (int i = 1; i < k_number; ++i) {
        int64_t next = stamps[i].micro_seconds_since_epoch();
        int64_t inc = next - start;
        start = next;
        if (inc < 0) {
            printf("reverse!\n");
        } else if (inc < 100) {
            ++increment[inc];
        } else {
            printf("big gap %d\n", static_cast<int>(inc));
        }
    }

    for (int i = 0; i < 100; ++i) {
        printf("%2d: %d\n", i, increment[i]);
    }

}

int main() {
    Timestamp now(Timestamp::now());
    printf("%s\n", now.to_string().c_str());
    pass_by_value(now);
    pass_by_const_reference(now);
    benchmark();
}