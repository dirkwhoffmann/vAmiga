// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RingBuffer.h"
#include "SchedulerTypes.h"

enum RegChangeID : i32
{
    SET_NONE,
    
    SET_BLTSIZE,
    SET_BLTSIZV,
    SET_BLTCON0,
    SET_BLTCON0L,
    SET_BLTCON1,
    
    SET_INTREQ,
    SET_INTENA,
    
    SET_BPLCON0_AGNUS,
    SET_BPLCON0_DENISE,
    SET_BPLCON1_AGNUS,
    SET_BPLCON1_DENISE,
    SET_BPLCON2,
    SET_BPLCON3,
    SET_DMACON,
    
    SET_DIWSTRT,
    SET_DIWSTOP,
    SET_DDFSTRT,
    SET_DDFSTOP,
    
    SET_BPL1MOD,
    SET_BPL2MOD,
    
    SET_SPR0DATA,
    SET_SPR1DATA,
    SET_SPR2DATA,
    SET_SPR3DATA,
    SET_SPR4DATA,
    SET_SPR5DATA,
    SET_SPR6DATA,
    SET_SPR7DATA,

    SET_SPR0DATB,
    SET_SPR1DATB,
    SET_SPR2DATB,
    SET_SPR3DATB,
    SET_SPR4DATB,
    SET_SPR5DATB,
    SET_SPR6DATB,
    SET_SPR7DATB,

    SET_SPR0POS,
    SET_SPR1POS,
    SET_SPR2POS,
    SET_SPR3POS,
    SET_SPR4POS,
    SET_SPR5POS,
    SET_SPR6POS,
    SET_SPR7POS,

    SET_SPR0CTL,
    SET_SPR1CTL,
    SET_SPR2CTL,
    SET_SPR3CTL,
    SET_SPR4CTL,
    SET_SPR5CTL,
    SET_SPR6CTL,
    SET_SPR7CTL,

    SET_BPL1PTH_1,
    SET_BPL2PTH_1,
    SET_BPL3PTH_1,
    SET_BPL4PTH_1,
    SET_BPL5PTH_1,
    SET_BPL6PTH_1,
    
    SET_BPL1PTH_2,
    SET_BPL2PTH_2,
    SET_BPL3PTH_2,
    SET_BPL4PTH_2,
    SET_BPL5PTH_2,
    SET_BPL6PTH_2,

    SET_BPL1PTL_1,
    SET_BPL2PTL_1,
    SET_BPL3PTL_1,
    SET_BPL4PTL_1,
    SET_BPL5PTL_1,
    SET_BPL6PTL_1,
    
    SET_BPL1PTL_2,
    SET_BPL2PTL_2,
    SET_BPL3PTL_2,
    SET_BPL4PTL_2,
    SET_BPL5PTL_2,
    SET_BPL6PTL_2,

    SET_SPR0PTH_1,
    SET_SPR1PTH_1,
    SET_SPR2PTH_1,
    SET_SPR3PTH_1,
    SET_SPR4PTH_1,
    SET_SPR5PTH_1,
    SET_SPR6PTH_1,
    SET_SPR7PTH_1,

    SET_SPR0PTH_2,
    SET_SPR1PTH_2,
    SET_SPR2PTH_2,
    SET_SPR3PTH_2,
    SET_SPR4PTH_2,
    SET_SPR5PTH_2,
    SET_SPR6PTH_2,
    SET_SPR7PTH_2,

    SET_SPR0PTL_1,
    SET_SPR1PTL_1,
    SET_SPR2PTL_1,
    SET_SPR3PTL_1,
    SET_SPR4PTL_1,
    SET_SPR5PTL_1,
    SET_SPR6PTL_1,
    SET_SPR7PTL_1,

    SET_SPR0PTL_2,
    SET_SPR1PTL_2,
    SET_SPR2PTL_2,
    SET_SPR3PTL_2,
    SET_SPR4PTL_2,
    SET_SPR5PTL_2,
    SET_SPR6PTL_2,
    SET_SPR7PTL_2,

    SET_DSKPTH_1,
    SET_DSKPTH_2,

    SET_DSKPTL_1,
    SET_DSKPTL_2,
        
    SET_STRHOR
};

/* Register change recorder
 *
 * For certain registers, Agnus and Denise have to keep track about when a
 * value changes. This information is stored in a sorted ring buffers called
 * a register change recorder.
 */
struct RegChange
{
    u32 addr;
    u16 value;
    u16 accessor;
    
    template <class W>
    void operator<<(W& worker)
    {
        worker << addr << value << accessor;
    }
    
    RegChange() : addr(0), value(0), accessor(0) { }
    RegChange(u32 a, u16 v) : addr(a), value(v), accessor(0) { }
    RegChange(u32 a, u16 v, u16 ac) : addr(a), value(v), accessor(ac) { }
};

template <isize capacity>
struct RegChangeRecorder : public util::SortedRingBuffer<RegChange, capacity>
{
    template <class W>
    void operator<<(W& worker)
    {
        worker >> this->elements << this->r << this->w << this->keys;
    }
    
    Cycle trigger() {
        return this->isEmpty() ? NEVER : this->keys[this->r];
    }
};
