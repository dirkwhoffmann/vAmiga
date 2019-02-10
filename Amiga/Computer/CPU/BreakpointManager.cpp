//
//  BreakpointManager.cpp
//  vAmiga
//
//  Created by Dirk Hoffmann on 06.02.19.
//  Copyright © 2019 Dirk Hoffmann. All rights reserved.
//

#include "Amiga.h"

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

bool
BreakpointManager::hasBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    return it != breakpoints.end();
}

bool
BreakpointManager::hasDisabledBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    return it != breakpoints.end() && !(*it).second.isEnabled();
}

bool
BreakpointManager::hasConditionalBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    return it != breakpoints.end() && (*it).second.hasCondition();
}

void
BreakpointManager::setBreakpointAt(uint32_t addr)
{
    amiga->suspend();
    
    breakpoints.insert(pair<uint32_t, Breakpoint>(addr, Breakpoint()));
    amiga->putMessage(MSG_BREAKPOINT);
    
    amiga->resume();
}

void
BreakpointManager::deleteBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    if (it == breakpoints.end()) return;
    
    amiga->suspend();
    
    breakpoints.erase(it);
    amiga->putMessage(MSG_BREAKPOINT);
    
    amiga->resume();
}

void
BreakpointManager::enableBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    if (it == breakpoints.end()) return;
    
    amiga->suspend();
    
    (*it).second.enable();
    amiga->putMessage(MSG_BREAKPOINT);
    
    amiga->resume();
}

void
BreakpointManager::disableBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    if (it == breakpoints.end()) return;
    
    amiga->suspend();
    
    (*it).second.disable();
    amiga->putMessage(MSG_BREAKPOINT);
    
    amiga->resume();
}

void
BreakpointManager::deleteBreakpoint(long nr)
{
    auto it = breakpoint(nr);
    if (it == breakpoints.end()) return;
    
    amiga->suspend();
    
    breakpoints.erase(it);
    amiga->putMessage(MSG_BREAKPOINT);
    
    amiga->resume();
}

uint32_t
BreakpointManager::getAddr(long nr)
{
    auto it = breakpoint(nr);
    if (it == breakpoints.end()) return UINT32_MAX;
    
    return (*it).first;
}

bool
BreakpointManager::setAddr(long nr, uint32_t addr)
{
    auto it = breakpoint(nr);
    if (it == breakpoints.end()) return false;
    
    amiga->suspend();
    
    Breakpoint bp = (*it).second;
    breakpoints.erase(it);
    breakpoints.insert(pair<uint32_t, Breakpoint>(addr, bp));
    amiga->putMessage(MSG_BREAKPOINT);
    
    amiga->resume();
    
    return true;
}

bool
BreakpointManager::isDisabled(long nr)
{
    auto it = breakpoint(nr);
    if (it == breakpoints.end()) return false;
    
    return !(*it).second.isEnabled();
}

bool
BreakpointManager::hasCondition(long nr)
{
    auto it = breakpoint(nr);
    if (it == breakpoints.end()) return false;
    
    return (*it).second.hasCondition();
}

bool
BreakpointManager::hasSyntaxError(long nr)
{
    auto it = breakpoint(nr);
    if (it == breakpoints.end()) return false;
  
    return (*it).second.hasSyntaxError();
}

const char *
BreakpointManager::getCondition(long nr)
{
    auto it = breakpoint(nr);
    if (it == breakpoints.end()) return "";
    
    return (*it).second.getCondition();
}

bool
BreakpointManager::setCondition(long nr, const char *str)
{
    auto it = breakpoint(nr);
    if (it == breakpoints.end()) return false;
    
    amiga->suspend();
    
    (*it).second.setCondition(str);
    amiga->putMessage(MSG_BREAKPOINT);
    
    amiga->resume();
    
    return true;
}

bool
BreakpointManager::removeCondition(long nr)
{
    auto it = breakpoint(nr);
    if (it == breakpoints.end()) return false;
    
    amiga->suspend();
    
    (*it).second.removeCondition();
    amiga->putMessage(MSG_BREAKPOINT);
    
    amiga->resume();
    
    return true;
}

bool
BreakpointManager::shouldStop()
{
    uint32_t addr = amiga->cpu.getPC();
    
    // Check if a soft breakpoint has been reached
    if (addr == softStop) {
        softStop = UINT32_MAX;
        return true;
    }
    
    // Check if a hard breakpoint has been reached
    auto it = breakpoints.find(addr);
    if (it != breakpoints.end()) {
        return (*it).second.eval();
    }
    
    return false;
}
