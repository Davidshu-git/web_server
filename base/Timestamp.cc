/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include <base/Timestamp.h>

#include <sys/time.h>
#include <cstdio>

namespace web_server {

string Timestamp::to_string() const {
    int64_t seconds = micro_seconds_since_epoch_ / k_micro_seconds_per_second;
    int64_t microseconds = micro_seconds_since_epoch_ % k_micro_seconds_per_second;
    return std::to_string(seconds) + "." + std::to_string(microseconds);
}

string Timestamp::to_formatted_string(bool show_micro_seconds) const {
    char buffer[64] = {0};
    time_t seconds = static_cast<time_t>(micro_seconds_since_epoch_ / k_micro_seconds_per_second);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
    if (show_micro_seconds) {
        int microseconds = static_cast<int>(micro_seconds_since_epoch_ / k_micro_seconds_per_second);
        snprintf(buffer, sizeof(buffer), "%4d%02d%02d %02d:%02d:%02d.%06d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
    } else {
        snprintf(buffer, sizeof(buffer), "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buffer;
}

Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * k_micro_seconds_per_second + tv.tv_usec);
}

} // namespace web_server