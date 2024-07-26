// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "RingBuffer.h"
#include "Serializable.h"
#include "AgnusTypes.h"
#include "AmigaTypes.h"
#include <functional>

namespace vamiga {

/* A key role in the architecture of vAmiga is played by two sorted ring
 * buffers:
 *
 * Register change recorder:
 *
 *     This buffer keeps track of all upcoming register changes. It is used
 *     to emulate the proper timing of all custom registers.
 *
 * Signal change recorder:
 *
 *     This buffer is used to emulate the display logic circuit. It keeps
 *     track of various signal changes such as the changes on the BPHSTART line
 *     that indicates a match of the horizontal counter with the DDF start
 *     position. The buffer is used to set up the bitplane events stored
 *     in the bplEvent table.
 */

//
// Register change recorder
//

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
    
    SET_DIWSTRT_AGNUS,
    SET_DIWSTRT_DENISE,
    SET_DIWSTOP_AGNUS,
    SET_DIWSTOP_DENISE,
    SET_DIWHIGH_AGNUS,
    SET_DIWHIGH_DENISE,
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

    SET_BPL1PTH,
    SET_BPL2PTH,
    SET_BPL3PTH,
    SET_BPL4PTH,
    SET_BPL5PTH,
    SET_BPL6PTH,
    
    SET_BPL1PTL,
    SET_BPL2PTL,
    SET_BPL3PTL,
    SET_BPL4PTL,
    SET_BPL5PTL,
    SET_BPL6PTL,
    
    SET_SPR0PTH,
    SET_SPR1PTH,
    SET_SPR2PTH,
    SET_SPR3PTH,
    SET_SPR4PTH,
    SET_SPR5PTH,
    SET_SPR6PTH,
    SET_SPR7PTH,

    SET_SPR0PTL,
    SET_SPR1PTL,
    SET_SPR2PTL,
    SET_SPR3PTL,
    SET_SPR4PTL,
    SET_SPR5PTL,
    SET_SPR6PTL,
    SET_SPR7PTL,

    SET_DSKPTH,
    SET_DSKPTL,

    SET_SERDAT
};

struct RegChange : Serializable
{
    u32 addr;
    u16 value;
    u16 accessor;


    //
    // Methods from Serializable
    //

public:

    template <class T>
    void serialize(T& worker)
    {
        worker

        << addr
        << value
        << accessor;

    } SERIALIZERS(serialize);


    RegChange() : addr(0), value(0), accessor(0) { }
    RegChange(u32 a, u16 v) : addr(a), value(v), accessor(0) { }
    RegChange(u32 a, u16 v, u16 ac) : addr(a), value(v), accessor(ac) { }
};

template <isize capacity>
struct RegChangeRecorder : public util::SortedRingBuffer<RegChange, capacity>
{
    Cycle trigger() {
        return this->isEmpty() ? NEVER : this->keys[this->r];
    }

    void apply(const std::function<void(i64 k, RegChange &e)> &func) {

        for (auto i = this->r; i != this->w; i = this->next(i)) {
            func(this->keys[i], this->elements[i]);
        }
    }

    void dump() {

        apply([] (i64 k, RegChange &e) {
            fprintf(stderr, "%lld: %s = %d\n", k, ChipsetRegEnum::key(e.addr), e.value);
        });
    }
};


//
// Signal change recorder
//

struct SigRecorder : public util::SortedArray<u32, 256>
{
    bool modified = false;
    
    SigRecorder& operator= (const SigRecorder& other) {

        SortedArray::operator = (other);
        CLONE(modified)

        return *this;
    }

    void insert(i64 key, u32 signal) {

        modified = true;
        
        for (isize i = 0; i < w; i++) {

            if (keys[i] == key) {
                elements[i] |= signal;
                return;
            }
        }
        
        SortedArray::insert(key, signal);
    }
    
    void invalidate(i64 key, u32 signal) {
        
        modified = true;
        
        for (isize i = 0; i < w; i++) {
            
            if ((elements[i] & signal) && keys[i] >= key) {
                elements[i] &= ~signal;
            }
        }
    }
};

}
