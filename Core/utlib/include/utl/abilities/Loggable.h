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

    // Additional prefix printed by logtrace()
    virtual void tracePrefix(const std::source_location &) const { };
};

}
