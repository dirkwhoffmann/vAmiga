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
#include "Copper.h"
#include "IO.h"

void
CopperDebugger::_reset(bool hard)
{
    cache.clear();
    current1 = nullptr;
    current2 = nullptr;
    dump();
}

void
CopperDebugger::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    os << "Cached elements:" << std::endl;
    
    for (auto &it: cache) {
                
        os << HEX32 << it.first << " -> (";
        os << HEX32 << it.second.start << ",";
        os << HEX32 << it.second.end << ")";
        os << std::endl;
    }
}

u32
CopperDebugger::startOfCopperList(isize nr)
{
    assert(nr == 1 || nr == 2);

    if (nr == 1) {
        return current1 ? current1->start : 0;
    } else {
        return current2 ? current2->start : 0;
    }
}

u32
CopperDebugger::endOfCopperList(isize nr)
{
    assert(nr == 1 || nr == 2);

    if (nr == 1) {
        return current1 ? current1->end : 0;
    } else {
        return current2 ? current2->end : 0;
    }
}

void
CopperDebugger::advanced()
{
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
