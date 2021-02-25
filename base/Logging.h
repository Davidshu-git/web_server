/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#ifndef WEB_SERVER_BASE_LOGGING_H
#define WEB_SERVER_BASE_LOGGING_H

#include <cstring>

#include "base/Timestamp.h"
#include "base/LogStream.h"

namespace web_server {

class Logger {
public:
    enum LogLevel {TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NUM_LOG_LEVELS};
    
    class SourceFile {
    public:
        template<int N>
        SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
            const char *slash = strrchr(data_, '/');
            if(slash) {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SourceFile(const char *filename) : data_(filename) {
            const char *slash = strrchr(data_, '/');
            if(slash) {
                data_ = slash + 1;
            }
            size_ = static_cast<int>(strlen(data_));
        }

        const char *data_;
        int size_;
    };
    Logger(SourceFile file, int line) : impl_(INFO, 0, file, line){};
    Logger(SourceFile file, int line, LogLevel level) : impl_(level, 0, file, line) {};
    Logger(SourceFile file, int line, LogLevel level, const char* func) : impl_(level, 0, file, line) {
        impl_.stream_ << func << ' ';
    };
    Logger(SourceFile file, int line, bool toAbort) : impl_(toAbort?FATAL:ERROR, errno, file, line) {};
    ~Logger();

    LogStream &stream() {
        return impl_.stream_;
    }

    static LogLevel log_level();
    static void set_log_level(LogLevel log_level);

    typedef void (*OutputFunc)(const char* msg, int len);
    typedef void (*FlushFunc)();
    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);

private:
    class Impl {
    public:
        using LogLevel = Logger::LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile &file, int line);
        void format_time();
        void finish();

        Timestamp time_;
        LogStream stream_;
        LogLevel level_;
        int line_;
        SourceFile basename_;
    };
    Impl impl_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::log_level() {
    return g_logLevel;
}

#define LOG_TRACE if (web_server::Logger::log_level() <= web_server::Logger::TRACE) \
    web_server::Logger(__FILE__, __LINE__, web_server::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (web_server::Logger::log_level() <= web_server::Logger::DEBUG) \
    web_server::Logger(__FILE__, __LINE__, web_server::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (web_server::Logger::log_level() <= web_server::Logger::INFO) \
    web_server::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN web_server::Logger(__FILE__, __LINE__, web_server::Logger::WARN).stream()
#define LOG_ERROR web_server::Logger(__FILE__, __LINE__, web_server::Logger::ERROR).stream()
#define LOG_FATAL web_server::Logger(__FILE__, __LINE__, web_server::Logger::FATAL).stream()
#define LOG_SYSERR web_server::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL web_server::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int saved_errno);

#define CHECK_NOTNULL(val) \
  ::web_server::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr) {
    if (ptr == NULL)
    {
    Logger(file, line, Logger::FATAL).stream() << names;
    }
    return ptr;
}

} // namespace web_server


#endif // WEB_SERVER_BASE_LOGGING_H