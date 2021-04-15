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

template <bool hires>
struct DDF
{
    // First bitplane DMA cycle of odd and even bitplanes
    i16 strt;

private:
    i16 strtEven;
public:
    
    // Last bitplane DMA cycle + 1 of odd and even bitplanes
    i16 stop;

private:
    i16 stopEven;
public:

    DDF() : strt(0), strtEven(0), stop(0), stopEven(0) { }
    
    template <class W>
    void operator<<(W& worker)
    {
        worker
        
        << strt
        << strtEven
        << stop
        << stopEven;
    }
    
    void clear() { strt = strtEven = stop = stopEven = 0; }
    
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
        
    /* Computes a DDF window
     *
     *       strt : Cycle number from DDFSTRT
     *       stop : Cycle number from DDFSTOP
     *     scroll : Scroll value from BPLCON1
     *
     * The function assumes that stop is greater than strt. Other combinations
     * are not yet supported by the emulator.
     */
    void compute(i16 ddfstrt, i16 ddfstop);
    void compute(i16 &strt, i16 &stop, i16 ddfstrt, i16 ddfstop);
};
