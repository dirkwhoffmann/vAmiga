// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CopperDebugger.h"
#include "Amiga.h"
#include "Copper.h"
#include "IO.h"

void
CopperDebugger::_reset(bool hard)
{
    cache.clear();
    current1 = nullptr;
    current2 = nullptr;
}

void
CopperDebugger::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;

    if (!amiga.inDebugMode()) {
        os << "No recorded data. Debug mode is off." << std::endl;
        return;
    }
    
    if ((category & dump::List1) && current1) {

        isize count = (current1->end - current1->start) / 4;
        for (isize i = 0; i < count && i < 100; i++) {
            os << string(disassemble(1, i)) << std::endl;
        }
    }
    
    if ((category & dump::List2) && current2) {

        isize count = (current2->end - current2->start) / 4;
        for (isize i = 0; i < count && i < 100; i++) {
            os << string(disassemble(2, i)) << std::endl;
        }
    }
}

u32
CopperDebugger::startOfCopperList(isize nr) const
{
    u32 result = 0;
    
    synchronized {

        assert(nr == 1 || nr == 2);

        if (nr == 1 && current1) result = current1->start;
        if (nr == 2 && current2) result = current2->start;
    }
    
    return result;
}

u32
CopperDebugger::endOfCopperList(isize nr) const
{
    u32 result = 0;
    
    synchronized {

        assert(nr == 1 || nr == 2);

        if (nr == 1 && current1) result = current1->end;
        if (nr == 2 && current2) result = current2->end;
    }
    
    return result;
}

void
CopperDebugger::advanced()
{
    synchronized {
        
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
}

void
CopperDebugger::jumped()
{
    synchronized {
        
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
}

string
CopperDebugger::disassemble(u32 addr) const
{
    char pos[16];
    char mask[16];
    char disassembly[128];
    
    if (copper.isMoveCmd(addr)) {
        
        sprintf(disassembly, "MOVE $%04X, %s", copper.getDW(addr), Memory::regName(copper.getRA(addr)));
        return string(disassembly);
    }
    
    const char *mnemonic = copper.isWaitCmd(addr) ? "WAIT" : "SKIP";
    const char *suffix = copper.getBFD(addr) ? "" : "b";
    
    sprintf(pos, "($%02X,$%02X)", copper.getVP(addr), copper.getHP(addr));
    
    if (copper.getVM(addr) == 0xFF && copper.getHM(addr) == 0xFF) {
        mask[0] = 0;
    } else {
        sprintf(mask, ", ($%02X,$%02X)", copper.getHM(addr), copper.getVM(addr));
    }
    
    sprintf(disassembly, "%s%s %s%s", mnemonic, suffix, pos, mask);
    return string(disassembly);
}

string
CopperDebugger::disassemble(isize list, isize offset) const
{
    assert(list == 1 || list == 2);
    
    u32 addr = (u32)((list == 1 ? copper.cop1lc : copper.cop2lc) + 2 * offset);
    return string(disassemble(addr));
}
