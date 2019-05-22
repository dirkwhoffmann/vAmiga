// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

BreakpointManager::BreakpointManager()
{
    setDescription("BreakpointManager");
    
    memset(breakpoints, 0, sizeof(breakpoints));
    numBreakpoints = 0;
}

Breakpoint *
BreakpointManager::breakpointWithNr(long nr)
{
    if (nr < numBreakpoints) {
        assert(breakpoints[nr] != NULL);
        return breakpoints[nr];
    }
    
    return NULL;
}

Breakpoint *
BreakpointManager::breakpointAtAddr(uint32_t addr)
{
    for (int i = 0; i < numBreakpoints; i++) {
        assert(breakpoints[i] != NULL);
        if (breakpoints[i]->addr == addr) return breakpoints[i];
    }
    
    return NULL;
}


/*
map<uint32_t, Breakpoint>::iterator
BreakpointManager::breakpoint(long nr)
{
    if (nr < breakpoints.size()) {
        auto it = breakpoints.begin();
        return next(it, nr);
    } else {
        return breakpoints.end();
    }
}
*/

bool
BreakpointManager::hasBreakpointAt(uint32_t addr)
{
    Breakpoint *bp = breakpointAtAddr(addr);
    
    return bp != NULL;
}

bool
BreakpointManager::hasDisabledBreakpointAt(uint32_t addr)
{
    Breakpoint *bp = breakpointAtAddr(addr);
    
    return bp != NULL && bp->isDisabled();
}

bool
BreakpointManager::hasConditionalBreakpointAt(uint32_t addr)
{
    Breakpoint *bp = breakpointAtAddr(addr);
    
    return bp != NULL && bp->hasCondition();
}

void
BreakpointManager::setBreakpointAt(uint32_t addr)
{
    debug("setBreakpointAt %X %d %d\n", addr, hasBreakpointAt(addr), numBreakpoints);
    
    if (!hasBreakpointAt(addr) && numBreakpoints + 1 < maxBreakpoints) {
        
        amiga->suspend();
        
        assert(breakpoints[numBreakpoints] == NULL);
        breakpoints[numBreakpoints] = new Breakpoint();
        breakpoints[numBreakpoints++]->addr = addr;
        
        amiga->putMessage(MSG_BREAKPOINT_CONFIG);
        
        amiga->resume();
    }
}

void
BreakpointManager::deleteBreakpoint(long nr)
{
    if (nr < numBreakpoints) {
        
        assert(breakpoints[nr] != NULL);
        deleteBreakpointAt(breakpoints[nr]->addr);
    }
}

void
BreakpointManager::deleteBreakpointAt(uint32_t addr)
{
    amiga->suspend();
    
    for (int i = 0; i < numBreakpoints; i++) {
        
        assert(breakpoints[i] != NULL);
        
        if (breakpoints[i]->addr == addr) {
            
            delete breakpoints[i];

            for (int j = i; j + 1 < numBreakpoints; j++)
                breakpoints[j] = breakpoints[j + 1];
            
            numBreakpoints--;
            breakpoints[numBreakpoints] = NULL;
        }
    }
    amiga->putMessage(MSG_BREAKPOINT_CONFIG);
    
    amiga->resume();
}

void
BreakpointManager::deleteAllBreakpoints()
{
    for (int i = 0; i < numBreakpoints; i++) {
        assert(breakpoints[i] != NULL);
        delete breakpoints[i];
        breakpoints[i] = NULL;
    }
    
    numBreakpoints = 0;
}

bool
BreakpointManager::isDisabled(long nr)
{
    Breakpoint *bp = breakpointWithNr(nr);
    return bp && bp->isDisabled();
}

void
BreakpointManager::setEnableAt(uint32_t addr, bool value)
{
    Breakpoint *bp = breakpointAtAddr(addr);
    if (!bp) return;
    
    amiga->suspend();
    
    value ? bp->enable() : bp->disable();
    amiga->putMessage(MSG_BREAKPOINT_CONFIG);
    
    amiga->resume();
}

uint32_t
BreakpointManager::getAddr(long nr)
{
    Breakpoint *bp = breakpointWithNr(nr);
    return bp ? bp->addr : UINT32_MAX;
}

bool
BreakpointManager::setAddr(long nr, uint32_t addr)
{
    Breakpoint *bp = breakpointWithNr(nr);
    if (!bp) return false;
    
    amiga->suspend();
    
    bp->addr = addr;
    amiga->putMessage(MSG_BREAKPOINT_CONFIG);
    
    amiga->resume();
    
    return true;
}

bool
BreakpointManager::hasCondition(long nr)
{
    Breakpoint *bp = breakpointWithNr(nr);
    return bp && bp->hasCondition();
}

bool
BreakpointManager::hasSyntaxError(long nr)
{
    Breakpoint *bp = breakpointWithNr(nr);
    return bp && bp->hasSyntaxError();
}

const char *
BreakpointManager::getCondition(long nr)
{
    Breakpoint *bp = breakpointWithNr(nr);
    return bp ? bp->getCondition() : "";
}

bool
BreakpointManager::setCondition(long nr, const char *str)
{
    Breakpoint *bp = breakpointWithNr(nr);
    if (!bp) return false;
    
    amiga->suspend();
    
    bp->setCondition(str);
    amiga->putMessage(MSG_BREAKPOINT_CONFIG);

    amiga->resume();
    
    return true;
}

bool
BreakpointManager::deleteCondition(long nr)
{
    Breakpoint *bp = breakpointWithNr(nr);
    if (!bp) return false;
    
    amiga->suspend();
    
    bp->removeCondition();
    amiga->putMessage(MSG_BREAKPOINT_CONFIG);

    amiga->resume();
    
    return true;
}

bool
BreakpointManager::shouldStop()
{
    uint32_t addr = amiga->cpu.getPC();
    
    // Check if a soft breakpoint has been reached.
    if (addr == softStop || softStop == UINT32_MAX) {
        
        // Soft breakpoints are deleted when reached.
        softStop = UINT32_MAX - 1;
        return true;
    }
    
    // Check if a hard breakpoint has been reached.
    Breakpoint *bp = breakpointAtAddr(addr);
    return bp ? bp->eval() : false;
}
