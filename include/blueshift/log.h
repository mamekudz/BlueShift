#pragma once

#include <cstdarg>
#include <cstdio>

#if defined(ESP_PLATFORM) && !defined(ARDUINO)
#include "esp_log.h"
#endif

// Structured logging categories + levels. Host-safe (stdio).
// Project categories remain the source of truth; ESP-IDF log ceiling mirrors setLogMaxLevel.

namespace blueshift {

enum class LogLevel : unsigned char {
    Error = 0,
    Warn = 1,
    Info = 2,
    Debug = 3,
    Trace = 4
};

#ifndef BLUESHIFT_LOG_MAX_LEVEL
#define BLUESHIFT_LOG_MAX_LEVEL 3
#endif

inline LogLevel &logMaxLevelRef() {
    static LogLevel level = static_cast<LogLevel>(BLUESHIFT_LOG_MAX_LEVEL);
    return level;
}

inline void setLogMaxLevel(LogLevel level) {
    logMaxLevelRef() = level;
#if defined(ESP_PLATFORM) && !defined(ARDUINO)
    // Keep ESP-IDF esp_log ceiling aligned with project level (single logging policy).
    esp_log_level_t idf = ESP_LOG_INFO;
    switch (level) {
    case LogLevel::Error:
        idf = ESP_LOG_ERROR;
        break;
    case LogLevel::Warn:
        idf = ESP_LOG_WARN;
        break;
    case LogLevel::Info:
        idf = ESP_LOG_INFO;
        break;
    case LogLevel::Debug:
        idf = ESP_LOG_DEBUG;
        break;
    case LogLevel::Trace:
        idf = ESP_LOG_VERBOSE;
        break;
    }
    esp_log_level_set("*", idf);
#endif
}

inline void logWrite(LogLevel level, const char *tag, const char *fmt, ...) {
    if (static_cast<unsigned>(level) > static_cast<unsigned>(logMaxLevelRef())) {
        return;
    }
    const char *lvl = "INFO";
    switch (level) {
    case LogLevel::Error:
        lvl = "ERROR";
        break;
    case LogLevel::Warn:
        lvl = "WARN";
        break;
    case LogLevel::Info:
        lvl = "INFO";
        break;
    case LogLevel::Debug:
        lvl = "DEBUG";
        break;
    case LogLevel::Trace:
        lvl = "TRACE";
        break;
    }
    std::printf("[%s][%s] ", tag, lvl);
    va_list args;
    va_start(args, fmt);
    std::vprintf(fmt, args);
    va_end(args);
    std::printf("\n");
}

} // namespace blueshift

#define BS_LOG_ERROR(tag, ...) ::blueshift::logWrite(::blueshift::LogLevel::Error, tag, __VA_ARGS__)
#define BS_LOG_WARN(tag, ...) ::blueshift::logWrite(::blueshift::LogLevel::Warn, tag, __VA_ARGS__)
#define BS_LOG_INFO(tag, ...) ::blueshift::logWrite(::blueshift::LogLevel::Info, tag, __VA_ARGS__)
#define BS_LOG_DEBUG(tag, ...) ::blueshift::logWrite(::blueshift::LogLevel::Debug, tag, __VA_ARGS__)
#define BS_LOG_TRACE(tag, ...) ::blueshift::logWrite(::blueshift::LogLevel::Trace, tag, __VA_ARGS__)
