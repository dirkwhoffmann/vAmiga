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

#include "utl/common.h"

namespace utl {

using LogChannel = isize;

struct LogChannelInfo {

    // Channel identifier
    string name;

    // Debug level (0 = no output)
    isize  level;

    // Optional description
    string description;
};

class Loggable {

    // Returns a reference to the channel pool
    static std::vector<LogChannelInfo> &channels();

public:

    // Looks up an existing channel or creates a new one if it does not exist
    static LogChannel subscribe(string name, isize level, string description = "");

    // Verbosity level passed to the prefix function
    static long verbosity;

    // Returns all registered channels
    static const std::vector<LogChannelInfo> &getChannels() noexcept { return channels(); }

    // Output functions (called by macro wrappers)
    __attribute__((format(printf, 4, 5)))
    void log(LogChannel channel, const std::source_location &loc, const char *fmt, ...) const;

    __attribute__((format(printf, 4, 5)))
    void traceLog(LogChannel channel, const std::source_location &loc, const char *fmt, ...) const;

    // Initializing
    Loggable() = default;
    virtual ~Loggable() = default;

protected:

    // Prefix printed prior to the debug message
    virtual void prefix(long verbosity, const std::source_location &) const { };

    // Additional prefix printed by trace()
    virtual void tracePrefix(long verbosity, const std::source_location &) const { };
};

#define msg(format, ...) { \
log(0, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__); }

#define warn(format, ...) { \
log(1, std::source_location::current(), "WARNING: " format __VA_OPT__(,) __VA_ARGS__); }

#define fatal(format, ...) { \
log(2, std::source_location::current(), "FATAL: " format __VA_OPT__(,) __VA_ARGS__); assert(false); exit(1); }

#define xfiles(format, ...) { \
log(3, std::source_location::current(), "XFILES: " format __VA_OPT__(,) __VA_ARGS__); }

#define debug(channel, format, ...) \
do { if ((channel) && verbosity) { \
log(channel, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__); \
}} while (0);

#define trace(channel, format, ...) \
do { if ((channel) && verbosity) { \
traceLog(channel, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__); \
}} while (0);

}
