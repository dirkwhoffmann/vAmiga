// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CopperDebugger.h"
#include "Amiga.h"
#include "Copper.h"
#include "IOUtils.h"

namespace vamiga {

void
CopperBreakpoints::setNeedsCheck(bool value)
{
    copper.checkForBreakpoints = value;
}

void
CopperWatchpoints::setNeedsCheck(bool value)
{
    copper.checkForWatchpoints = value;
}

void
CopperDebugger::_reset(bool hard)
{
    cache.clear();
    current1 = nullptr;
    current2 = nullptr;
}

void
CopperDebugger::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    auto print = [&](const string &name, const moira::Guards &guards) {

        for (int i = 0; i < guards.elements(); i++) {

            auto bp =  guards.guardNr(i);
            auto nr = name + std::to_string(i);

            os << tab(nr);
            os << hex(bp->addr);

            if (!bp->enabled) os << " (Disabled)";
            else if (bp->ignore) os << " (Disabled for " << dec(bp->ignore) << " hits)";
            os << std::endl;
        }
    };

    if (!amiga.isTracking()) {
        
        os << "No recorded data. Debug mode is off." << std::endl;
        return;
    }
    
    if (category == Category::List1 && current1) {

        isize count = (current1->end - current1->start) / 4;
        for (isize i = 0; i < count && i < 100; i++) {
            os << string(disassemble(1, i, true)) << std::endl;
        }
    }
    
    if (category == Category::List2 && current2) {

        isize count = (current2->end - current2->start) / 4;
        for (isize i = 0; i < count && i < 100; i++) {
            os << string(disassemble(2, i, true)) << std::endl;
        }
    }
    
    if (category == Category::Breakpoints) {

        if (breakpoints.elements()) {
            print("Breakpoint", breakpoints);
        } else {
            os << "No breakpoints set" << std::endl;
        }
    }

    if (category == Category::Watchpoints) {

        if (watchpoints.elements()) {
            print("Watchpoint", watchpoints);
        } else {
            os << "No watchpoints set" << std::endl;
        }
    }
}

u32
CopperDebugger::startOfCopperList(isize nr) const
{
    assert(nr == 1 || nr == 2);

    SYNCHRONIZED
    
    u32 result = 0;
    
    if (nr == 1 && current1) result = current1->start;
    if (nr == 2 && current2) result = current2->start;
    
    return result;
}

u32
CopperDebugger::endOfCopperList(isize nr) const
{
    assert(nr == 1 || nr == 2);

    SYNCHRONIZED

    u32 result = 0;

    if (nr == 1 && current1) result = current1->end;
    if (nr == 2 && current2) result = current2->end;
    
    return result;
}

void
CopperDebugger::advanced()
{
    SYNCHRONIZED
    
    auto addr = copper.coppc;
    auto nr = copper.copList;
    assert(nr == 1 || nr == 2);
    
    // Adjust the end address if the Copper went beyond
    if (nr == 1 && current1 && current1->end < addr) {
        current1->end = addr;
    }
    if (nr == 2 && current2 && current2->end < addr) {
        current2->end = addr;
    }
}

void
CopperDebugger::jumped()
{
    SYNCHRONIZED
    
    auto addr = copper.coppc;
    auto nr = copper.copList;
    assert(nr == 1 || nr == 2);
    
    // Lookup Copper list in cache
    auto list = cache.find(addr);
    
    // Create a new list if it was not found
    if (list == cache.end()) {
        cache.insert(std::make_pair(addr, CopperList { addr, addr }));
        list = cache.find(addr);
    }
    
    // Switch to the new list
    if (nr == 1) {
        current1 = &list->second;
    } else {
        current2 = &list->second;
    }
}

string
CopperDebugger::disassemble(isize list, isize offset, bool symbolic) const
{
    assert(list == 1 || list == 2);
    
    u32 addr = (u32)((list == 1 ? copper.cop1lc : copper.cop2lc) + 2 * offset);
    return string(disassemble(addr, symbolic));
}

string
CopperDebugger::disassemble(u32 addr, bool symbolic) const
{
    if (symbolic) {
        
        char pos[16];
        char mask[16];
        char txt[128];
        
        if (copper.isMoveCmd(addr)) {
            
            auto source = copper.getDW(addr);
            auto target = Debugger::regName(copper.getRA(addr));
            snprintf(txt, sizeof(txt), "MOVE $%04X, %s", source, target);
            
            return string(txt);
        }
        
        const char *mnemonic = copper.isWaitCmd(addr) ? "WAIT" : "SKIP";
        const char *suffix = copper.getBFD(addr) ? "" : "b";
        
        auto vp = copper.getVP(addr);
        auto hp = copper.getHP(addr);
        snprintf(pos, sizeof(pos), "($%02X,$%02X)", vp, hp);
        
        if (copper.getVM(addr) == 0xFF && copper.getHM(addr) == 0xFF) {
            mask[0] = 0;
        } else {
            
            auto hm = copper.getHM(addr);
            auto vm = copper.getVM(addr);
            snprintf(mask, sizeof(mask), ", ($%02X,$%02X)", hm, vm);
        }
        
        snprintf(txt, sizeof(txt), "%s%s %s%s", mnemonic, suffix, pos, mask);
        return string(txt);
        
    } else {
        
        auto word1 = mem.spypeek16 <ACCESSOR_AGNUS> (addr);
        auto word2 = mem.spypeek16 <ACCESSOR_AGNUS> (addr + 2);
        
        auto hex1 = util::hexstr <4> (word1);
        auto hex2 = util::hexstr <4> (word2);
        
        return "dc.w " + hex1 + "," + hex2;
    }
}

void
CopperDebugger::setBreakpoint(u32 addr, isize ignores)
{
    if (breakpoints.isSetAt(addr)) throw VAError(ERROR_BP_ALREADY_SET, addr);

    breakpoints.setAt(addr, ignores);
    msgQueue.put(MSG_COPPERBP_UPDATED);
}

void
CopperDebugger::deleteBreakpoint(isize nr)
{
    if (!breakpoints.isSet(nr)) throw VAError(ERROR_BP_NOT_FOUND, nr);

    breakpoints.remove(nr);
    msgQueue.put(MSG_COPPERBP_UPDATED);
}

void
CopperDebugger::enableBreakpoint(isize nr)
{
    if (!breakpoints.isSet(nr)) throw VAError(ERROR_BP_NOT_FOUND, nr);

    breakpoints.enable(nr);
    msgQueue.put(MSG_COPPERBP_UPDATED);
}

void
CopperDebugger::disableBreakpoint(isize nr)
{
    if (!breakpoints.isSet(nr)) throw VAError(ERROR_BP_NOT_FOUND, nr);

    breakpoints.disable(nr);
    msgQueue.put(MSG_COPPERBP_UPDATED);
}

void
CopperDebugger::toggleBreakpoint(isize nr)
{
    breakpoints.isEnabled(nr) ? disableBreakpoint(nr) : enableBreakpoint(nr);
}

void
CopperDebugger::ignoreBreakpoint(isize nr, isize count)
{
    if (!breakpoints.isSet(nr)) throw VAError(ERROR_BP_NOT_FOUND, nr);

    breakpoints.ignore(nr, count);
    msgQueue.put(MSG_COPPERBP_UPDATED);
}

void
CopperDebugger::setWatchpoint(u32 addr, isize ignores)
{
    if (watchpoints.isSetAt(addr)) throw VAError(ERROR_WP_ALREADY_SET, addr);

    watchpoints.setAt(addr, ignores);
    msgQueue.put(MSG_COPPERWP_UPDATED);
}

void
CopperDebugger::deleteWatchpoint(isize nr)
{
    if (!watchpoints.isSet(nr)) throw VAError(ERROR_WP_NOT_FOUND, nr);

    watchpoints.remove(nr);
    msgQueue.put(MSG_COPPERWP_UPDATED);
}

void
CopperDebugger::enableWatchpoint(isize nr)
{
    if (!watchpoints.isSet(nr)) throw VAError(ERROR_WP_NOT_FOUND, nr);

    watchpoints.enable(nr);
    msgQueue.put(MSG_COPPERWP_UPDATED);
}

void
CopperDebugger::toggleWatchpoint(isize nr)
{
    watchpoints.isEnabled(nr) ? disableWatchpoint(nr) : enableWatchpoint(nr);
}

void
CopperDebugger::disableWatchpoint(isize nr)
{
    if (!watchpoints.isSet(nr)) throw VAError(ERROR_WP_NOT_FOUND, nr);

    watchpoints.disable(nr);
    msgQueue.put(MSG_COPPERWP_UPDATED);
}

void
CopperDebugger::ignoreWatchpoint(isize nr, isize count)
{
    if (!watchpoints.isSet(nr)) throw VAError(ERROR_WP_NOT_FOUND, nr);

    watchpoints.ignore(nr, count);
    msgQueue.put(MSG_COPPERWP_UPDATED);
}

}
