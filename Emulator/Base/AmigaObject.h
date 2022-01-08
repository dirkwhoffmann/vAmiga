// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

/* Object model:
 *
 * ------------------
 * |  AmigaObject   |
 * ------------------
 *         |
 * ------------------
 * | AmigaComponent |
 * ------------------
 *         |
 *         |   ------------------   ---------------------   ----------------
 *         |-->|     Thread     |-->| SuspendableThread |-->|    Amiga     |
 *         |   ------------------   ---------------------   ----------------
 *         |   ------------------
 *         |-->|  SubComponent  |
 *             ------------------
 *
 * AmigaObject is the base class for all Amiga related classes. It provides a
 * a textual description for the object as well as various functions for
 * printing debug information.
 *
 * AmigaComponent defines the base functionality of all hardware components. It
 * comprises functions for initializing, configuring, and serializing the
 * object, as well as functions for powering up and down, running and
 * pausing. Furthermore, a 'synchronized' macro is provided to prevent mutual
 * execution of certain code components.
 *
 * Thread adds the ability to run the component asynchroneously. It implements
 * the emulator's state model (off, paused, running). SuspendableThread extends
 * the Thread class with the suspend/resume mechanism which can be utilized to
 * pause the emulator temporarily.
 */

#include "Error.h"

namespace dump {
enum Category : usize {
    
    BankMap   = 0b0000000001,
    Checksums = 0b0000000010,
    Config    = 0b0000000100,
    Dma       = 0b0000001000,
    List1     = 0b0000010000,
    List2     = 0b0000100000,
    Registers = 0b0001000000,
    Segments  = 0b0010000000,
    State     = 0b0100000000,
    Signals   = 0b1000000000
};
}

class AmigaObject {

    //
    // Initializing
    //

public:

    virtual ~AmigaObject() { };
    
    
    //
    // Printing debug information
    //
    
    // Returns the name for this component (e.g., "Agnus" or "Denise")
    virtual const char *getDescription() const = 0;
    
    // Called by debug() and trace() to produce a detailed debug output
    virtual void prefix() const;
    
    // Prints debug information about this component
    void dump(dump::Category category, std::ostream& ss) const;
    void dump(dump::Category category) const;
    void dump(std::ostream& ss) const;
    void dump() const;
    virtual void _dump(dump::Category category, std::ostream& ss) const = 0;
};

/* This file provides several macros for printing messages:
 *
 *   - msg    Information message   (Shows up in all builds)
 *   - warn   Warning message       (Shows up in all builds)
 *   - fatal  Error message + Exit  (Shows up in all builds)
 *   - debug  Debug message         (Shows up in debug builds, only)
 *   - plain  Plain debug message   (Shows up in debug builds, only)
 *   - trace  Detailed debug output (Shows up in debug builds, only)
 *
 * Debug messages are prefixed by the component name and a line number. Trace
 * messages are prefixed by a more detailed string description produced by the
 * prefix() function.
 *
 * Debug, plain, and trace messages are accompanied by an optional 'verbose'
 * parameter. If 0 is passed in, no output will be generated.
 *
 * Sidenote: In previous releases the printing macros were implemented in form
 * of variadic functions. Although this might seem to be superior at first
 * glance, it is not. Using macros allows modern compilers to verify the format
 * strings against the data types of the provided arguments. This check can't
 * be performed when variadic functions are used.
 */

#define msg(format, ...) \
fprintf(stderr, format, ##__VA_ARGS__);

#define warn(format, ...) \
fprintf(stderr, "Warning: " format, ##__VA_ARGS__);

#define fatal(format, ...) \
{ fprintf(stderr, "Fatal: " format, ##__VA_ARGS__); exit(1); }

#ifndef NDEBUG

#define debug(verbose, format, ...) \
if constexpr (verbose) { \
fprintf(stderr, "%s:%d " format, getDescription(), __LINE__, ##__VA_ARGS__); }

#define plain(verbose, format, ...) \
if constexpr (verbose) { \
fprintf(stderr, format, ##__VA_ARGS__); }

#define trace(verbose, format, ...) \
if constexpr (verbose) { \
prefix(); \
fprintf(stderr, "%s:%d " format, getDescription(), __LINE__, ##__VA_ARGS__); }

#else

#define debug(verbose, format, ...)
#define plain(verbose, format, ...)
#define trace(verbose, format, ...)

#endif
