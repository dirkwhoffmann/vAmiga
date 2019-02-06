// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CPU_INC
#define _CPU_INC

#include "BreakpointManager.h"

//
// THIS CLASS IS A STUB TO MAKE THE VISUAL PROTOTYPE WORK
//

class CPU : public HardwareComponent {
    
public:
    
    // A breakpoint manager used for debugging
    BreakpointManager bpManager;
    
    
    // DEPRECATED
    map<uint32_t, Breakpoint> breakpoints;
    
public:
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    CPU();
    ~CPU();
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
   
public:
    
    // Collects the data shown in the GUI's debug panel
    CPUInfo getInfo();
    
    //
    // Managing breakpoints
    //
    
    long numberOfBreakpoints() { return breakpoints.size(); }
    
    bool hasBreakpointAt(uint32_t addr);
    bool hasConditionalBreakpointAt(uint32_t addr);
    void addBreakpointAt(uint32_t addr);
    void deleteBreakpointAt(uint32_t addr);
    void toggleBreakpointAt(uint32_t addr);
    
    bool hasCondition(long nr);
    bool hasSyntaxError(long nr);
    uint32_t getBreakpointAddr(long nr);
    bool setBreakpointAddr(long nr, uint32_t addr);
    const char *getBreakpointCondition(long nr);
    bool setBreakpointCondition(long nr, const char *cond);

    /*
    - (BOOL) newBreakpointWithCondition:(NSString *)cond;
    - (BOOL) setBreakpointAddr:(NSInteger)nr addr:(NSInteger)addr;
    - (BOOL) deleteBreakpoint:(NSInteger)nr;
    
 */
    
    
    
    //
    // FAKE METHODS FOR THE VISUAL PROTOTYPE (TEMPORARY)
    //
    
public:
    
    uint64_t executeNextInstruction();
    
    void disassemble();
    
    
 
    
};

#endif
