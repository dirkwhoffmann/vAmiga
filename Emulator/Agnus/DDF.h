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
    // First bitplane DMA cycle
    i16 strt;
    
    // Last bitplane DMA cycle + 1
    i16 stop;

    DDF() : strt(0), stop(0) { }
    DDF(u16 strt, i16 stop) : strt(strt), stop(stop) { }

    template <class T> void applyToItems(T& worker) { worker & strt & stop; }
    
    DDF& operator=(const DDF& ddf)
    {
        strt = ddf.strt;
        stop = ddf.stop;
        return *this;
    }

    bool operator==(const DDF& ddf) const
    {
        return strt == ddf.strt && stop == ddf.stop;
    }

    bool operator!=(const DDF& ddf) const
    {
        return strt != ddf.strt || stop != ddf.stop;
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
    void compute(i16 ddfstrt, i16 ddfstop, u8 scroll);
};

#endif
