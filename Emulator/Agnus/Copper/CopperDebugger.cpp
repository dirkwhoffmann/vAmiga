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
    current = nullptr;
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
CopperDebugger::startOfCopperList()
{
    return current ? current->start : 0;
}

u32
CopperDebugger::endOfCopperList()
{
    return current ? current->end : 0;
}

void
CopperDebugger::advanced()
{
    u32 addr = copper.coppc;
    
    // Adjust the end address if the Copper went beyond
    if (current && current->end < addr) {
        current->end = addr;
    }
}

void
CopperDebugger::jumped()
{
    u32 addr = copper.coppc;
    
    // Lookup Copper list in cache
    auto list = cache.find(addr);

    // Create a new list if it was not found
    if (list == cache.end()) {
        cache.insert(std::make_pair(addr, CopperList { addr, addr }));
        list = cache.find(addr);
    }
    
    // Switch to the new list
    current = &list->second;
}
