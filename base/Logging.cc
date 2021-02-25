/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include "base/Logging.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "base/CurrentThread.h"
#include "base/Timestamp.h"

namespace web_server {

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

const char* strerror_tl(int saved_errno) {
    return strerror_r(saved_errno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel init_log_level() {
    if (::getenv("WEB_SERVER_LOG_TRACE"))
        return Logger::TRACE;
    else if (::getenv("WEB_SERVER_LOG_DEBUG"))
        return Logger::DEBUG;
    else
        return Logger::INFO;
}

Logger::LogLevel g_logLevel = init_log_level();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {"TRACE ", "DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL "};

class T {
public:
    T(const char* str, unsigned len) : str_(str), len_(len) {
        assert(strlen(str) == len_);
    }
    const char* str_;
    const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v) {
    s.append(v.str_, v.len_);
    return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v) {
    s.append(v.data_, v.size_);
    return s;
}

void defaultOutput(const char* msg, int len) {
    size_t n = fwrite(msg, 1, len, stdout);
}

void defaultFlush() {
    fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

Logger::Impl::Impl(LogLevel level, int old_errno, const SourceFile &file, int line) 
    : time_(Timestamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file) {
    format_time();
    web_server::current_thread::tid();
    stream_ << T(current_thread::tid_string(), current_thread::tid_string_length());
    stream_ << T(LogLevelName[level], 6);
    if (old_errno != 0) {
        stream_ << strerror_tl(old_errno) << " (errno=" << old_errno << ") ";
    }
}

void Logger::Impl::format_time() {
    int64_t micro_seconds_since_epoch = time_.micro_seconds_since_epoch();
    time_t seconds = static_cast<time_t>(micro_seconds_since_epoch / Timestamp::k_micro_seconds_per_second);
    int micro_seconds = static_cast<int>(micro_seconds_since_epoch % Timestamp::k_micro_seconds_per_second);
    if (seconds != t_lastSecond) {
        t_lastSecond = seconds;
        struct tm tm_time;
        ::gmtime_r(&seconds, &tm_time);
        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d.%06d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, micro_seconds);
        assert(len == 24);
    }
    stream_ << T(t_time, 24) << " ";
}

void Logger::Impl::finish() {
    stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::~Logger() {
    impl_.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (impl_.level_ == FATAL) {
        g_flush();
        abort();
    }
}

void Logger::set_log_level(Logger::LogLevel level) {
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out) {
  g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
  g_flush = flush;
}

} // namespace web_server