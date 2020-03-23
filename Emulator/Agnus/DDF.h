// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DDF_INC
#define _DDF_INC

#include <sys/param.h>
#include "Aliases.h"

template <bool hires>
struct DDF
{
    // First bitplane DMA cycle of even and odd bitplanes
    i16 strtEven;
    i16 strtOdd;
    
    // Last bitplane DMA cycle + 1 of even and odd bitplanes
    i16 stopEven;
    i16 stopOdd;
    
    DDF() : strtEven(0), strtOdd(0), stopEven(0), stopOdd(0) { }

    template <class T> void applyToItems(T& worker) {
        
        worker
        
        & strtEven
        & strtOdd
        & stopEven
        & stopOdd;
    }
    
    void clear() { strtEven = strtOdd = stopEven = stopOdd = 0; }
    
    /*
    bool operator==(const DDF& ddf) const
    {
        return strt == ddf.strt && stop == ddf.stop;
    }

    bool operator!=(const DDF& ddf) const
    {
        return strt != ddf.strt || stop != ddf.stop;
    }
    */
    
    /* Computes a DDF window
     *
     *       strt : Cycle number from DDFSTRT
     *       stop : Cycle number from DDFSTOP
     *     scroll : Scroll value from BPLCON1
     *
     * The function assumes that stop is greater than strt. Other combinations
     * are not yet supported by the emulator.
     */
    void compute(i16 ddfstrt, i16 ddfstop, u16 bplcon1);
    void compute(i16 &strt, i16 &stop, i16 ddfstrt, i16 ddfstop, int scroll);
};

#endif
