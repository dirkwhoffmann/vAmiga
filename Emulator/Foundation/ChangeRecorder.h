// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Commons.h"
#include "AmigaPublicTypes.h"
#include "RingBuffer.h"


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

    template <class T>
    void applyToItems(T& worker)
    {
        worker & addr & value;
    }

    RegChange() : addr(0), value(0) { }
    RegChange(u32 a, u16 v) : addr(a), value(v) { }
};

template <isize capacity>
struct RegChangeRecorder : public utl::SortedRingBuffer<RegChange, capacity>
{
    template <class W>
    void applyToItems(W& worker)
    {
        worker & this->elements & this->r & this->w & this->keys;
    }
    
    Cycle trigger() {
        return this->isEmpty() ? NEVER : this->keys[this->r];
    }
};

