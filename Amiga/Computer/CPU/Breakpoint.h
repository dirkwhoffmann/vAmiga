// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _BREAKPOINT_INC
#define _BREAKPOINT_INC

#include "va_std.h"
#include <vector>
#include <string>

using std::vector;
using std::string;

class ASTNode;

class Breakpoint {
  
private:
    
    // The breakpoint's address in memory
    uint32_t addr;
    
    /* Indicates if this breakpoint is a soft breakpoint
     * In contrast to standard (hard) breakpoints, soft breakpoints are deleted
     * when reached. They are only used internally to implement manual stepping
     * in the CPU debug panel.
     */
    bool soft;
    
    /* The breakpoint's condition
     * This variable points to the root node in the condition's AST. It is
     * NULL if the breakpoint is unconditional (default).
     */
    ASTNode *condition = NULL;
    
public:
    
    Breakpoint(uint32_t addr, bool soft = false);
    
    /* Sets a breakpoint condition
     *
     * The condition is provided by a string conforming to the following
     * (left-recursive) grammar:
     *
     *       <BOOL> ::= <BOOL1> [ '||' <BOOL> ]
     *      <BOOL1> ::= '!' <BOOL> | <BOOL2> [ '&&' <BOOL1> ]
     *      <BOOL2> ::= <ATOMIC> | (<BOOL>)
     *
     *     <ATOMIC> ::= <VALUE> <COMP> <VALUE>
     *       <COMP> ::= '==' | '!=' | '<' | '>'
     *
     *      <VALUE> ::= <REGISTER> | <DIRECT> | <INDIRECT>
     *   <REGISTER> ::= ['D0' - 'D7', 'A0' - 'A7']
     *     <DIRECT> ::= '$'[a-f|A-F|0-9]+ | [0-9]+
     *   <INDIRECT> ::= '(' <VALUE> ')'
     *
     * Returns true if the condition was parsed successfully.
     */
    bool setCondition(const char *str);
    
    /* Deletes a breakpoint condition
     */
    void removeCondition();
};

#endif
