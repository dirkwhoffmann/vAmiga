// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include <source_location>

/* This file provides several macros for printing messages:
 *
 *   - msg    Information message   (Shows up in all builds)
 *   - warn   Warning message       (Shows up in all builds)
 *   - fatal  Error message + Exit  (Shows up in all builds)
 *   - debug  Debug message         (Shows up in debug builds, only)
 *   - trace  Detailed debug output (Shows up in debug builds, only)
 *
 * Debug, plain, and trace messages are accompanied by an optional 'enable'
 * parameter. If 0 is passed in, no output will be generated. In addition,
 * variable 'verbose' is checked which is set to true by default. By setting
 * this variable to false, debug output can be silenced temporarily.
 *
 * Debug messages are also affected by the verbosity level which is a static
 * member of the CoreComponent class. If set to 0, all debug messages are
 * omitted. If set the 1, debug messages appear as plain text. If set to a
 * value of 1 or above, the debug message is prefixed with additional
 * information about the emulator state, such as the component name issuing
 * the message, the currently processed frame, or the value of CPU flags.
 *
 * Sidenote: In previous releases the printing macros were implemented in form
 * of variadic functions. Although this might seem to be superior at first
 * glance, it is not. Using macros allows modern compilers to verify the format
 * strings against the data types of the provided arguments. This check can't
 * be performed when variadic functions are used.
 */

#include "utl/abilities/Reflectable.h"

namespace utl {

using LogChannel = isize;

enum class LogLevel : long
{
    LV_EMERGENCY   = 0,
    LV_ALERT       = 1,
    LV_CRITICAL    = 2,
    LV_ERROR       = 3,
    LV_WARNING     = 4,
    LV_NOTICE      = 5,
    LV_INFO        = 6,
    LV_DEBUG       = 7
};

struct LogLevelEnum : Reflectable<LogLevelEnum, LogLevel>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = (long)LogLevel::LV_DEBUG;

    static const char *_key(long value) { return _key(LogLevel(value)); }
    static const char *_key(LogLevel value)
    {
        switch (value) {

            case LogLevel::LV_EMERGENCY:    return "LV_EMERGENCY";
            case LogLevel::LV_ALERT:        return "LV_ALERT";
            case LogLevel::LV_CRITICAL:     return "LV_CRITICAL";
            case LogLevel::LV_ERROR:        return "LV_ERROR";
            case LogLevel::LV_WARNING:      return "LV_WARNING";
            case LogLevel::LV_NOTICE:       return "LV_NOTICE";
            case LogLevel::LV_INFO:         return "LV_INFO";
            case LogLevel::LV_DEBUG:        return "LV_DEBUG";
        }
        return "???";
    }
    static const char *help(long value) { return help(LogLevel(value)); }
    static const char *help(LogLevel value)
    {
        switch (value) {

            case LogLevel::LV_EMERGENCY:    return "System is unusable";
            case LogLevel::LV_ALERT:        return "Immediate action required";
            case LogLevel::LV_CRITICAL:     return "Critical condition";
            case LogLevel::LV_ERROR:        return "Error condition";
            case LogLevel::LV_WARNING:      return "Warning condition";
            case LogLevel::LV_NOTICE:       return "Normal but significant condition";
            case LogLevel::LV_INFO:         return "Informational message";
            case LogLevel::LV_DEBUG:        return "Debug message";
        }
        return "???";
    }
};

struct LogChannelInfo {

    // Channel identifier
    string name;

    // Severity level
    optional<LogLevel> level;

    // Optional description
    string description;
};

class Loggable {

    // Returns a reference to the channel pool
    static std::vector<LogChannelInfo> &channels();

public:

    // Looks up an existing channel or creates a new one if it does not exist
    static LogChannel subscribe(string name, optional<long> level, string description = "");
    static LogChannel subscribe(string name, optional<LogLevel> level, string description = "");

    // Modifies the verbosity of an existing channel
    static void setLevel(isize nr, optional<LogLevel> level);
    static void setLevel(string name, optional<LogLevel> level);

    // Returns the number of registered channels
    static isize size() noexcept { return channels().size(); }

    // Returns all registered channels
    static const std::vector<LogChannelInfo> &getChannels() noexcept { return channels(); }

    // Output functions (called by macro wrappers)
    __attribute__((format(printf, 5, 6))) void log(LogChannel channel,
                                                   LogLevel level,
                                                   const std::source_location &loc,
                                                   const char *fmt, ...) const;

    // Initializing
    Loggable() = default;
    virtual ~Loggable() = default;

protected:

    // Prefix printed prior to the debug message
    virtual void prefix(const std::source_location &) const { };

    // Additional prefix printed by trace()
    virtual void tracePrefix(const std::source_location &) const { };
};

#define CONCAT(a,b) a##b
#define LOG_CHANNEL(a) CONCAT(CH_,a)

#define msg(format, ...) { \
log(1, LogLevel::LV_NOTICE, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__); }

#define warn(format, ...) { \
log(1, LogLevel::LV_WARNING, std::source_location::current(), "WARNING: " format __VA_OPT__(,) __VA_ARGS__); }

#define fatal(format, ...) { \
log(1, LogLevel::LV_EMERGENCY, std::source_location::current(), "FATAL: " format __VA_OPT__(,) __VA_ARGS__); assert(false); exit(1); }

#define xfiles(format, ...) { \
log(CH_XFILES, LogLevel::LV_NOTICE, std::source_location::current(), "XFILES: " format __VA_OPT__(,) __VA_ARGS__); }

#ifdef NDEBUG

#define debug(channel, format, ...) \
do { if constexpr (channel) { \
log(LOG_CHANNEL(channel), LogLevel::LV_INFO, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__); \
}} while (0);

#define trace(channel, format, ...) \
do { if constexpr (channel) { \
log(LOG_CHANNEL(channel), LogLevel::LV_DEBUG, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__); \
}} while (0);

#else

#define debug(channel, format, ...) \
log(LOG_CHANNEL(channel), LogLevel::LV_INFO, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__);

#define trace(channel, format, ...) \
log(LOG_CHANNEL(channel), LogLevel::LV_DEBUG, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__);

#endif
}
