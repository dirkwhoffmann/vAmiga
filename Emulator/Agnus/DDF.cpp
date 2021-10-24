// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DDF.h"
#include <algorithm>

template <bool hires> void
DDF<hires>::compute(isize ddfstrt, isize ddfstop)
{
    if constexpr (hires) {
               
        // Compute the beginning of the fetch window
        strt = ddfstrt & ~0b11;
        
        // Compute the number of fetch units
        isize fetchUnits = ((ddfstop - ddfstrt) + 15) >> 3;
        
        // The number of fetch units can't be negative
        if (fetchUnits < 0) fetchUnits = 0;
        
        // Compute the end of the DDF window
        stop = std::min(strt + 8 * fetchUnits, (isize)0xE0);
        
    } else {
        
        // Compute the beginning of the fetch window
        strt = ddfstrt & ~0b11;

        // Compute the number of fetch units
        isize fetchUnits = ((ddfstop - ddfstrt) + 15) >> 3;
        
        // The number of fetch units can't be negative
        if (fetchUnits < 0) fetchUnits = 0;

        // Compute the end of the DDF window
        stop = std::min(strt + 8 * fetchUnits, (isize)0xE0);
    }
}

template void DDF<true>::compute(isize ddfstrt, isize ddfstop);
template void DDF<false>::compute(isize ddfstrt, isize ddfstop);
