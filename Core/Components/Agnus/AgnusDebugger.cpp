// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "AgnusDebugger.h"
#include "Amiga.h"
#include "Emulator.h"
#include "utl/io.h"

namespace vamiga {

void
AgnusDebugger::_dump(Category category, std::ostream &os) const
{
    using namespace utl;

    if (category == Category::Beamtraps) {

        if (beamtraps.elements()) {

            for (int i = 0; i < beamtraps.elements(); i++) {

                auto bp = *beamtraps.guardNr(i);
                auto v = std::to_string(HI_WORD(bp.addr));
                auto h = std::to_string(LO_WORD(bp.addr));
                os << tab("Beamtrap " + std::to_string(i));
                os << "(" + v + "," + h + ")";

                if (!bp.enabled) os << " (Disabled)";
                else if (bp.ignore) os << " (Disabled for " << dec(bp.ignore) << " hits)";
                os << std::endl;
            }

        } else {

            os << "No beamtraps set" << std::endl;
        }
    }
}

void
AgnusDebugger::eolHandler()
{
    // Check if execution should be interrupted
    if (eolTrap) { eolTrap = false; amiga.setFlag(RL::EOL_REACHED); }
}

void
AgnusDebugger::eofHandler()
{
    // Check if execution should be interrupted
    if (eofTrap) { eofTrap = false; amiga.setFlag(RL::EOF_REACHED); }
}

}
