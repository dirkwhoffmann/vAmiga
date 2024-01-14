// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"
#include "IOUtils.h"

namespace vamiga {

void
Copper::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::List1 || category == Category::List2) {

        debugger.dump(category, os);
    }

    if (category == Category::Registers) {

        auto deltaPC = coppc - coppc0;

        os << tab("COPPC");
        os << hex(coppc0) << " ( +" << dec(deltaPC) << " )" << std::endl;
        os << tab("COP1LC");
        os << hex(cop1lc) << std::endl;
        os << tab("COP1LC");
        os << hex(cop2lc) << std::endl;
        os << tab("COPINS1");
        os << hex(cop1ins) << std::endl;
        os << tab("COPINS2");
        os << hex(cop2ins) << std::endl;
        os << tab("CDANG");
        os << bol(cdang) << std::endl;
    }

    if (category == Category::State) {

        os << tab("Active Copper list");
        os << dec(copList) << std::endl;
        os << tab("Skip flag");
        os << bol(skip) << std::endl;
    }
}

void
Copper::_inspect() const
{
    SYNCHRONIZED
    
    info.copList = copList;
    info.copList1Start = debugger.startOfCopperList(1);
    info.copList1End = debugger.endOfCopperList(1);
    info.copList2Start = debugger.startOfCopperList(2);
    info.copList2End = debugger.endOfCopperList(2);
    info.active = agnus.isPending<SLOT_COP>();
    info.cdang = cdang;
    info.coppc0 = coppc0 & agnus.ptrMask;
    info.cop1lc = cop1lc & agnus.ptrMask;
    info.cop2lc = cop2lc & agnus.ptrMask;
    info.cop1ins = cop1ins;
    info.cop2ins = cop2ins;
}

}
