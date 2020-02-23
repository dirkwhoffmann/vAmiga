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

#include "AmigaUtils.h"

#include <vector>
#include <map>

using std::vector;
using std::map;
using std::pair;
using std::swap;

/* Base class for all vAmiga objects.
 * This class contains a textual description of the object and offers various
 * functions for printing debug messages and warnings.
 */
class AmigaObject {
    
    public:
    
    // Debug level for this component
    unsigned debugLevel = 1;
    
    private:
    
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
    const char *getDescription() const { return description ? description : ""; }
    void setDescription(const char *str) { description = strdup(str); }
    
    
    //
    // Debugging the component
    //
    
    protected:
    
    /* There a four types of messages:
     *
     *   - msg     Debug message   (Shows up in debug and release builds)
     *   - warn    Warning message (Does not terminate the program)
     *   - panic   Error message   (Terminates the program)
     *   - debug   Debug message   (Shows up in debug builds, only)
     *
     * Debug messages are prefixed by the string description produced by the
     * prefix() function. To omit the prefix, use plaindebug(...) instead.
     * Some Amiga objects overwrite prefix() to provide addition standard debug
     * information when a message is printed.
     */
    virtual void prefix();
    
    void msg(const char *fmt, ...);
    void warn(const char *fmt, ...);
    void panic(const char *fmt, ...);

    void debug(const char *fmt, ...);
    void debug(int level, const char *fmt, ...);
    void plaindebug(const char *fmt, ...);
    void plaindebug(int level, const char *fmt, ...);
};

#endif
