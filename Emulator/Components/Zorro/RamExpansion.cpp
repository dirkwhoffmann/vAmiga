// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RamExpansion.h"
#include "Memory.h"

namespace vamiga {

void
RamExpansion::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    ZorroBoard::_dump(category, os);
}

void
RamExpansion::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {

        // Enter autoconfig state, if FastRam should be emulated
        state = mem.fastRamSize() ? STATE_AUTOCONF : STATE_SHUTUP;
    }
}

bool
RamExpansion::pluggedIn() const
{
    return mem.fastRamSize() != 0;
}

isize
RamExpansion::pages() const
{
    return mem.fastRamSize() / KB(64);
}

void
RamExpansion::updateMemSrcTables()
{
    isize numPages = mem.getConfig().fastSize / 0x10000;
    isize firstPage = baseAddr / 0x10000;

    // Only proceed if the board has been configured
    if (firstPage == 0) return;

    for (isize i = firstPage; i < firstPage + numPages; i++) {

        mem.cpuMemSrc[i] = MEM_FAST;
    }
}

}
