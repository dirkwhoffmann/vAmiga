// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RingBuffer.h"
#include "Serializable.h"
#include "AgnusTypes.h"
#include "AmigaTypes.h"
#include "MemoryTypes.h"
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

struct RegChange : Serializable
{
    Reg reg;
    u16 value;
    Accessor accessor;


    //
    // Methods from Serializable
    //

public:

    template <class T>
    void serialize(T& worker)
    {
        worker

        << reg
        << value
        << accessor;

    } SERIALIZERS(serialize);
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
            fprintf(stderr, "%lld: %s = %d\n", k, RegEnum::key(e.reg), e.value);
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
