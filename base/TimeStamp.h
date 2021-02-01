/**
 * @brief time stamp file
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_BASE_TIMESTAMP_H
#define WEB_SERVER_BASE_TIMESTAMP_H

#include <ctime>
#include <boost/operators.hpp>

#include "base/Types.h"
#include "base/Copyable.h"

namespace web_server {

class Timestamp : public Copyable, public boost::equality_comparable<Timestamp>, public boost::less_than_comparable<Timestamp> {
public:
    Timestamp() : micro_seconds_since_epoch_(0) {}
    explicit Timestamp(int64_t micro_seconds_since_epoch)
        : micro_seconds_since_epoch_(micro_seconds_since_epoch) {}

    void swap(Timestamp &object) {
        std::swap(micro_seconds_since_epoch_, object.micro_seconds_since_epoch_);
    }

    string to_string() const;
    string to_formatted_string(bool show_micro_seconds = true) const;

    bool valid() const {
        return micro_seconds_since_epoch_ > 0;
    }
    int64_t micro_seconds_since_epoch() const {
        return micro_seconds_since_epoch_;
    }

    static const int k_micro_seconds_per_second = 1000 * 1000;

    time_t seconds_since_epoch() const {
        return static_cast<time_t>(micro_seconds_since_epoch_ / k_micro_seconds_per_second);
    }

    static Timestamp now();

    static Timestamp invalid() {
        return Timestamp();
    }

    static Timestamp form_unix_time(time_t seconds, int micro_seconds) {
        return Timestamp(static_cast<int64_t>(seconds) * k_micro_seconds_per_second + micro_seconds);
    }

private:
    int64_t micro_seconds_since_epoch_;
};

} // namespace web_server

#endif // WEB_SERVER_BASE_TIMESTAMP_H