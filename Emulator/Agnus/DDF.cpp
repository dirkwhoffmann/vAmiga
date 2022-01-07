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

void
DDF::compute(isize ddfstrt, isize ddfstop)
{
    // Compute the beginning of the fetch window
    strt = ddfstrt;
    
    // Compute the number of fetch units
    isize fetchUnits = ((ddfstop - ddfstrt) + 15) >> 3;
    
    // The number of fetch units can't be negative
    if (fetchUnits < 0) fetchUnits = 0;
    
    // Compute the end of the DDF window
    stop = std::min(strt + 8 * fetchUnits, (isize)0xE0);
}
