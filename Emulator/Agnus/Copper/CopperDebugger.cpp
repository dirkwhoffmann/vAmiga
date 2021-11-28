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
#include "IOUtils.h"

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
    char txt[128];
    
    if (copper.isMoveCmd(addr)) {
        
        auto source = copper.getDW(addr);
        auto target = Memory::regName(copper.getRA(addr));
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
}

string
CopperDebugger::disassemble(isize list, isize offset) const
{
    assert(list == 1 || list == 2);
    
    u32 addr = (u32)((list == 1 ? copper.cop1lc : copper.cop2lc) + 2 * offset);
    return string(disassemble(addr));
}
