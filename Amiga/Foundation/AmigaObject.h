// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGAOBJECT_INC
#define _AMIGAOBJECT_INC

#include "vastd.h"

/* Base class for all vAmiga objects.
 * This class stores a textual description of the object and offers various
 * functions for printing debug messages and warnings.
 */
class AmigaObject {
    
public:
    
    // Debug level for this component
    unsigned debugLevel = DEBUG_LEVEL;
    
private:
    
    /* Each component has the built-in ability to print trace messages.
     * Such messages are used during debugging, e.g., to print something after
     * each processed CPU instruction. To avoid flooding the console, trace
     * messages are only printed if this counter hasn't reached 0 yet. It is
     * set in startTracing() and decremented each time a trace message is
     * printed. Setting this variable to a negative value has the effect of
     * tracing forever.
     */
    long traceCounter = 0;
    
    /* Textual description of this object
     * Most debug output methods preceed their output with this string.
     * The default value is NULL. In that case, no prefix is printed.
     */
    const char *description = NULL;
    
    
    //
    // Initializing the component
    //
    
public:
    
    // Getter and setter for the textual description.
    const char *getDescription() { return description ? description : ""; }
    void setDescription(const char *str) { description = strdup(str); }
    
    
    //
    // Debugging the component
    //
    
public:
    
    // Returns if trace mode is enabled.
    bool tracingEnabled() { return traceCounter != 0; }
    
    // Starts tracing.
    void startTracing(int counter = -1) { traceCounter = counter; }
    
    // Stops tracing.
    void stopTracing() { traceCounter = 0; }
    
    // Prints a trace message if tracing is enabled
    void trace();
    
private:
    
    /* Prints the actual trace message to the console
     * This method is overwritten by sub-classes that support tracing.
     */
    virtual void _trace() { }
    
    //
    // Printing messages to the console
    //
    
protected:
    
    /* There a four types of messages:
     *
     *   - msg     Debug message   (Shows up in debug and release build)
     *   - debug   Debug message   (Shows up in debug build, only)
     *   - warn    Warning message (Does not terminate the program)
     *   - panic   Error message   (Terminates the program)
     *
     * All messages are prefixed by the string description of the printing
     * object. To omit the prefix, use plainmsg or plaindebug instead.
     */
    void msg(const char *fmt, ...);
    void plainmsg(const char *fmt, ...);
    
    void debug(const char *fmt, ...);
    void debug(int level, const char *fmt, ...);
    void plaindebug(const char *fmt, ...);
    void plaindebug(int level, const char *fmt, ...);

    void warn(const char *fmt, ...);
    void panic(const char *fmt, ...);
};

#endif
