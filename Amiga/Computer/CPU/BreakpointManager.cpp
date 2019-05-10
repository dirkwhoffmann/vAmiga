// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

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
    _amiga->suspend();
    
    breakpoints.insert(pair<uint32_t, Breakpoint>(addr, Breakpoint()));
    _amiga->putMessage(MSG_BREAKPOINT_CONFIG);
    
    _amiga->resume();
}

void
BreakpointManager::deleteBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    if (it == breakpoints.end()) return;
    
    _amiga->suspend();
    
    breakpoints.erase(it);
    _amiga->putMessage(MSG_BREAKPOINT_CONFIG);
    
    _amiga->resume();
}

void
BreakpointManager::enableBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    if (it == breakpoints.end()) return;
    
    _amiga->suspend();
    
    (*it).second.enable();
    _amiga->putMessage(MSG_BREAKPOINT_CONFIG);
    
    _amiga->resume();
}

void
BreakpointManager::disableBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    if (it == breakpoints.end()) return;
    
    _amiga->suspend();
    
    (*it).second.disable();
    _amiga->putMessage(MSG_BREAKPOINT_CONFIG);
    
    _amiga->resume();
}

void
BreakpointManager::deleteBreakpoint(long nr)
{
    auto it = breakpoint(nr);
    if (it == breakpoints.end()) return;
    
    _amiga->suspend();
    
    breakpoints.erase(it);
    _amiga->putMessage(MSG_BREAKPOINT_CONFIG);
    
    _amiga->resume();
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
    
    _amiga->suspend();
    
    Breakpoint bp = (*it).second;
    breakpoints.erase(it);
    breakpoints.insert(pair<uint32_t, Breakpoint>(addr, bp));
    _amiga->putMessage(MSG_BREAKPOINT_CONFIG);
    
    _amiga->resume();
    
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
    
    _amiga->suspend();
    
    (*it).second.setCondition(str);
    _amiga->putMessage(MSG_BREAKPOINT_CONFIG);
    
    _amiga->resume();
    
    return true;
}

bool
BreakpointManager::removeCondition(long nr)
{
    auto it = breakpoint(nr);
    if (it == breakpoints.end()) return false;
    
    _amiga->suspend();
    
    (*it).second.removeCondition();
    _amiga->putMessage(MSG_BREAKPOINT_CONFIG);
    
    _amiga->resume();
    
    return true;
}

bool
BreakpointManager::shouldStop()
{
    uint32_t addr = _cpu->getPC();
    
    // Check if a soft breakpoint has been reached
    if (addr == softStop || softStop == UINT32_MAX) {
        softStop = UINT32_MAX - 1;
        return true;
    }
    
    // Check if a hard breakpoint has been reached
    auto it = breakpoints.find(addr);
    if (it != breakpoints.end()) {
        return (*it).second.eval();
    }
    
    return false;
}
