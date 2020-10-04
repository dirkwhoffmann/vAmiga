// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_OBJECT_H
#define _AMIGA_OBJECT_H

#include "Utils.h"

#include <vector>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

using std::vector;
using std::map;
using std::pair;
using std::swap;

#define synchronized \
    for(std::unique_lock<std::recursive_mutex> _l(mutex); _l; _l.unlock())

/* Base class for all Amiga objects. This class contains a textual description
 * of the object and offers various functions for printing debug messages and
 * warnings.
 */
class AmigaObject {
    
public:
    
    virtual ~AmigaObject() { };
    
private:
    
    /* Textual description of this object. Most debug output methods preceed
     * their output with this string. If set to NULL, no prefix is printed.
     */
    const char *description = NULL;
    
protected:
    
    /* Mutex for implementing the 'synchronized' macro. The macro can be used
     * to prevent multiple threads to enter the same code block. It mimics the
     * behaviour of the well known Java construct 'synchronized(this) { }'.
     */
    std::recursive_mutex mutex;
    
    
    //
    // Initializing
    //
    
public:
    
    const char *getDescription() const { return description ? description : ""; }
    void setDescription(const char *str) { description = strdup(str); }
    
    
    //
    // Debugging the component
    //
    
protected:
    
    /* There a four types of messages:
     *
     *   - msg     Debug messages   (Show up in debug and release builds)
     *   - warn    Warning messages (Do not terminate the program)
     *   - panic   Error messages   (Terminate the program)
     *   - debug   Debug messages   (Show up in debug builds, only)
     *
     * Debug messages are prefixed by the string description produced by the
     * prefix() function. To omit the prefix, use plaindebug(...). Some objects
     * overwrite prefix() to output additional debug information.
     */
    virtual void prefix() { };
    
    void msg(const char *fmt, ...);
    void warn(const char *fmt, ...);
    void panic(const char *fmt, ...);
    
    void debug(const char *fmt, ...);
    void debug(int verbose, const char *fmt, ...);
    void plaindebug(const char *fmt, ...);
    void plaindebug(int verbose, const char *fmt, ...);
};

#endif
