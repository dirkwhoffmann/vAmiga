// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

struct DDF
{
    // First bitplane DMA cycle of odd and even bitplanes
    isize strt;
    
    // Last bitplane DMA cycle + 1 of odd and even bitplanes
    isize stop;

    DDF() : strt(0), stop(0) { }
    
    template <class W>
    void operator<<(W& worker)
    {
        worker
        
        << strt
        << stop;
    }
    
    void clear() { strt = stop = 0; }
    
    bool operator==(const DDF& ddf) const
    {
        return
        strt == ddf.strt &&
        stop == ddf.stop;
    }

    bool operator!=(const DDF& ddf) const
    {
        return !(*this == ddf);
    }
        
    bool inside(isize posh) const
    {
        return posh >= strt && posh < stop;
    }
    
    /* Computes a DDF window. The function assumes that ddfstop is greater than
     * ddfstrt. Other combinations are not yet supported by the emulator.
     * DEPRECATED
     */
    void compute(isize ddfstrt, isize ddfstop);
};
