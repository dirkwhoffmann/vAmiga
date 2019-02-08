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
#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

class ASTNode;

class Breakpoint {
  
private:
    
    // Indicates if this breakpoint is enabled
    bool enabled = true;
    
    /* The breakpoint's condition as it was entered by the user
     */
    string conditionStr = "";
    
    /* The breakpoint's condition translated to an AST.
     */
    ASTNode *ast = NULL;
    
public:
    
    // Returns true if this breakpoint is enabled.
    bool isEnabled() { return enabled; }
    
    // Enables a disabled breakpoint and vice versa.
    void toggleDisabled() { enabled = !enabled; }
    
    // Returns true if this is an coditional breakpoint.
    bool hasCondition() { return ast != NULL; }
 
    // Returns true if the condition contains a syntax error.
    bool hasSyntaxError() { return conditionStr != "" && ast == NULL; }
    
    // Returns a textual description of the breakpoint condition.
    const char *getCondition();
    
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
     *       <COMP> ::= '==' | '!=' | '<' | '<=' | '>' | '>='
     *
     *      <VALUE> ::= <REGISTER> | <DIRECT> | <INDIRECT>
     *   <REGISTER> ::= ['D0' - 'D7', 'A0' - 'A7']
     *     <DIRECT> ::= <DEC> | <HEX>
     *        <DEC> ::= [0-9]+
     *        <HEX> ::= '$'[a-fA-F0-9]+
     *   <INDIRECT> ::= '(' <VALUE> ').'[bwl]
     *
     * Returns true if the condition has been parsed successfully.
     */
    bool setCondition(const char *str);
    
    // Deletes a breakpoint condition
    bool removeCondition();
    
    // Evaluates a breakpoint
    bool eval();
};

#endif
