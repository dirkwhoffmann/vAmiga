// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "LogicAnalyzer.h"
#include "Amiga.h"

namespace vamiga {

/*
LogicAnalyzer::LogicAnalyzer(Amiga &ref) : SubComponent(ref)
{
    
}
*/

void
LogicAnalyzer::_didReset(bool hard)
{
    servicePROEvent();
}

void
LogicAnalyzer::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {
        
        dumpConfig(os);
    }
}

void
LogicAnalyzer::servicePROEvent()
{
    
}

}
