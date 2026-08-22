// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

/* The Loggable interface provides a framework for printing log messages.
 *
 * Messages are generated via the log function and are always written to
 * stderr. Whether a call site actually calls log() at all is decided at
 * the call site itself (see the logging macros in debug.h), based on a
 * per-flag debug setting rather than a runtime channel lookup.
 */

#pragma once

#include "utl/abilities/Reflectable.h"
#include <source_location>

namespace utl {

enum class LogLevel : long
{
    LOG_EMERG   = 0,
    LOG_ALERT   = 1,
    LOG_CRIT    = 2,
    LOG_ERR     = 3,
    LOG_WARNING = 4,
    LOG_NOTICE  = 5,
    LOG_INFO    = 6,
    LOG_DEBUG   = 7
};

struct LogLevelEnum : Reflectable<LogLevelEnum, LogLevel>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = (long)LogLevel::LOG_DEBUG;

    static const char *_key(long value) { return _key(LogLevel(value)); }
    static const char *_key(LogLevel value)
    {
        switch (value) {

            case LogLevel::LOG_EMERG:   return "LV_EMERGENCY";
            case LogLevel::LOG_ALERT:   return "LV_ALERT";
            case LogLevel::LOG_CRIT:    return "LV_CRITICAL";
            case LogLevel::LOG_ERR:     return "LV_ERROR";
            case LogLevel::LOG_WARNING: return "LV_WARNING";
            case LogLevel::LOG_NOTICE:  return "LV_NOTICE";
            case LogLevel::LOG_INFO:    return "LV_INFO";
            case LogLevel::LOG_DEBUG:   return "LV_DEBUG";
        }
        return "???";
    }
    static const char *help(long value) { return help(LogLevel(value)); }
    static const char *help(LogLevel value)
    {
        switch (value) {

            case LogLevel::LOG_EMERG:   return "System is unusable";
            case LogLevel::LOG_ALERT:   return "Immediate action required";
            case LogLevel::LOG_CRIT:    return "Critical condition";
            case LogLevel::LOG_ERR:     return "Error condition";
            case LogLevel::LOG_WARNING: return "Warning condition";
            case LogLevel::LOG_NOTICE:  return "Normal but significant condition";
            case LogLevel::LOG_INFO:    return "Informational message";
            case LogLevel::LOG_DEBUG:   return "Debug message";
        }
        return "???";
    }
};

class Loggable {

public:

    // Output function (called by macro wrappers)
#if defined(__clang__)
    __attribute__((format(printf, 4, 5)))
#endif
    void log(LogLevel level,
             const std::source_location &loc,
             const char *fmt, ...) const;

    // Initializing
    Loggable() = default;
    virtual ~Loggable() = default;

protected:

    // Optional prefix printed prior to the debug message
    virtual string prefix(LogLevel, const std::source_location &) const;
};

}
