// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

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

namespace vamiga {

class Loggable {

public:

    // Verbosity level
    static long verbosity;

    virtual ~Loggable() = default;

protected:

    // Called by debug() and trace() to produce a detailed debug output
    virtual void prefix(long verbosity, const void *sender, long line) const { };
};

#define msg(format, ...) \
fprintf(stderr, format __VA_OPT__(,) __VA_ARGS__);

#define warn(format, ...) \
fprintf(stderr, "Warning: " format __VA_OPT__(,) __VA_ARGS__);

#define fatal(format, ...) \
{ fprintf(stderr, "Fatal: " format __VA_OPT__(,) __VA_ARGS__); assert(false); exit(1); }

#define debug(enable, format, ...) \
if (enable) { if (verbosity) { \
prefix(verbosity, this, __LINE__); \
fprintf(stderr, format __VA_OPT__(,) __VA_ARGS__); }}

#define trace(enable, format, ...) \
if (enable) { if (verbosity) { \
prefix(5, this, __LINE__); \
fprintf(stderr, format __VA_OPT__(,) __VA_ARGS__); }}

#define xfiles(format, ...) \
if (XFILES) { if (verbosity) { \
prefix(verbosity, this, __LINE__); \
fprintf(stderr, "XFILES: " format __VA_OPT__(,) __VA_ARGS__); }}

}
