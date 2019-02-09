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
    breakpoints.insert(pair<uint32_t, Breakpoint>(addr, Breakpoint()));
    amiga->putMessage(MSG_BREAKPOINT);
}

void
BreakpointManager::deleteBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    if (it != breakpoints.end()) {
        breakpoints.erase(it);
        amiga->putMessage(MSG_BREAKPOINT);
    }
}

void
BreakpointManager::enableBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    if (it != breakpoints.end()) {
        (*it).second.enable();
        amiga->putMessage(MSG_BREAKPOINT);
    }
}

void
BreakpointManager::disableBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    if (it != breakpoints.end()) {
        (*it).second.disable();
        amiga->putMessage(MSG_BREAKPOINT);
    }
}

/*
void
BreakpointManager::enableOrDisableBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    if (it != breakpoints.end()) {
        (*it).second.toggleDisabled();
        amiga->putMessage(MSG_BREAKPOINT);
    }
}
*/

/*
void
BreakpointManager::toggleBreakpointAt(uint32_t addr)
{
    if (hasBreakpointAt(addr)) {
        deleteBreakpointAt(addr);
    } else {
        addBreakpointAt(addr);
    }
}
*/

void
BreakpointManager::deleteBreakpoint(long nr)
{
    if (nr >= breakpoints.size())
        return;
    
    auto it = breakpoints.begin();
    advance(it, nr);
    breakpoints.erase(it);
    amiga->putMessage(MSG_BREAKPOINT);
}

uint32_t
BreakpointManager::getAddr(long nr)
{
    if (nr >= breakpoints.size())
        return UINT32_MAX;
    
    auto it = breakpoints.begin();
    advance(it, nr);
    return (*it).first;
}

bool
BreakpointManager::setAddr(long nr, uint32_t addr)
{
    if (nr >= breakpoints.size())
        return false;
    
    auto it = breakpoints.begin();
    advance(it, nr);
    Breakpoint bp = (*it).second;
    breakpoints.erase(it);
    breakpoints.insert(pair<uint32_t, Breakpoint>(addr, bp));
    amiga->putMessage(MSG_BREAKPOINT);
    return true;
}

bool
BreakpointManager::isDisabled(long nr)
{
    auto i = breakpoint(nr);
    if (i != breakpoints.end()) {
        return !(*i).second.isEnabled();
    }
    return false;
}

bool
BreakpointManager::hasCondition(long nr)
{
    auto i = breakpoint(nr);
     if (i != breakpoints.end()) {
        return (*i).second.hasCondition();
     }
    return false; 
}

bool
BreakpointManager::hasSyntaxError(long nr)
{
    auto i = breakpoint(nr);
    if (i != breakpoints.end()) {
        return (*i).second.hasSyntaxError();
    }
    return false;
}

const char *
BreakpointManager::getCondition(long nr)
{
    auto i = breakpoint(nr);
    if (i != breakpoints.end()) {
        return (*i).second.getCondition();
    }
    return "";
}

bool
BreakpointManager::setCondition(long nr, const char *str)
{
    auto i = breakpoint(nr);
    if (i != breakpoints.end()) {
        (*i).second.setCondition(str);
        amiga->putMessage(MSG_BREAKPOINT);
        return true;
    }
    return false;
}

bool
BreakpointManager::removeCondition(long nr)
{
    auto i = breakpoint(nr);
    if (i != breakpoints.end()) {
        (*i).second.removeCondition();
        amiga->putMessage(MSG_BREAKPOINT);
        return true;
    }
    return false;
}
