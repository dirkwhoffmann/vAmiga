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
BreakpointManager::hasConditionalBreakpointAt(uint32_t addr)
{
    auto it = breakpoints.find(addr);
    return it != breakpoints.end() && (*it).second.hasCondition();
}

void
BreakpointManager::addBreakpointAt(uint32_t addr)
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
BreakpointManager::toggleBreakpointAt(uint32_t addr)
{
    if (hasBreakpointAt(addr)) {
        deleteBreakpointAt(addr);
    } else {
        addBreakpointAt(addr);
    }
}


