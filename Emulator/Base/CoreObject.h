// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Error.h"
#include "Dumpable.h"

namespace vamiga {

class CoreObject : public Dumpable {

protected:

    /* Verbosity level.
     *
     * The variable controls the amount and overall format of generated debug
     * output. If set to zero, all debug messages are omitted. If set to a
     * value of 1 or above, the debug message is prefixed with additional
     * information about the emulator state, such as the component name issuing
     * the message, the currently processed frame, or the value of CPU flags.
     */
    static isize verbosity;

    
    //
    // Initializing
    //

public:

    virtual ~CoreObject() = default;

    // Returns the name for this component
    virtual const char *objectName() const = 0;

    // Returns a textual description for this component
    virtual const char *description() const { return ""; }
    
    // Called by debug() and trace() to produce a detailed debug output
    virtual void prefix(isize level, isize line) const;
};

/* This file provides several macros for printing messages:
 *
 *   - msg    Information message   (Shows up in all builds)
 *   - warn   Warning message       (Shows up in all builds)
 *   - fatal  Error message + Exit  (Shows up in all builds)
 *   - debug  Debug message         (Shows up in debug builds, only)
 *   - trace  Detailed debug output (Shows up in debug builds, only)
 *
 * Debug messages are prefixed by the component name and a line number. Trace
 * messages are prefixed by a more detailed string description produced by the
 * prefix() function.
 *
 * Debug, plain, and trace messages are accompanied by an optional 'enable'
 * parameter. If 0 is passed in, no output will be generated. In addition,
 * variable 'verbose' is checked which is set to true by default. By setting
 * this variable to false, debug output can be silenced temporarily.
 *
 * Sidenote: In previous releases the printing macros were implemented in form
 * of variadic functions. Although this might seem to be superior at first
 * glance, it is not. Using macros allows modern compilers to verify the format
 * strings against the data types of the provided arguments. This check can't
 * be performed when variadic functions are used.
 */

#define msg(format, ...) \
fprintf(stderr, format __VA_OPT__(,) __VA_ARGS__);

#define warn(format, ...) \
fprintf(stderr, "Warning: " format __VA_OPT__(,) __VA_ARGS__);

#define fatal(format, ...) \
{ fprintf(stderr, "Fatal: " format __VA_OPT__(,) __VA_ARGS__); exit(1); }

#define debugmsg(enable, level, format, ...) \
if (enable) { if (verbosity) { \
prefix(level, __LINE__); \
fprintf(stderr, format __VA_OPT__(,) __VA_ARGS__); }}

#define debug(enable, format, ...) \
debugmsg(enable, verbosity, format __VA_OPT__(,) __VA_ARGS__);

// DEPRECATED
#define trace(enable, format, ...) \
debugmsg(enable, 5, format __VA_OPT__(,) __VA_ARGS__);

#define xfiles(format, ...) \
if (XFILES) { if (verbosity) { \
prefix(verbosity, __LINE__); \
fprintf(stderr, "XFILES: " format __VA_OPT__(,) __VA_ARGS__); }}

}
