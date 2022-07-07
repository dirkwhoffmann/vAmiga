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

#ifdef __cplusplus

static constexpr u16 SIG_NONE           = 0b0000000000;
static constexpr u16 SIG_CON            = 0b0000000001;
static constexpr u16 SIG_BMAPEN_CLR     = 0b0000000010;
static constexpr u16 SIG_BMAPEN_SET     = 0b0000000100;
static constexpr u16 SIG_VFLOP_CLR      = 0b0000001000;
static constexpr u16 SIG_VFLOP_SET      = 0b0000010000;
static constexpr u16 SIG_BPHSTART       = 000000100000;
static constexpr u16 SIG_BPHSTOP        = 0b0001000000;
static constexpr u16 SIG_SHW            = 0b0010000000;
static constexpr u16 SIG_RHW            = 0b0100000000;
static constexpr u16 SIG_DONE           = 0b1000000000;

struct DDFState
{
    bool bpv = false;
    bool bmapen = false;
    bool shw = false;
    bool rhw = false;
    bool bphstart = false;
    bool bphstop = false;
    bool bprun = false;
    bool lastFu = false;
    bool stopreq = false;
    u16  bplcon0 = 0;
    u8   cnt = 0;

    bool operator==(const DDFState &rhs) const
    {
        return
        this->bpv == rhs.bpv &&
        this->bmapen == rhs.bmapen &&
        this->shw == rhs.shw &&
        this->rhw == rhs.rhw &&
        this->bphstart == rhs.bphstart &&
        this->bphstop == rhs.bphstop &&
        this->bprun == rhs.bprun &&
        this->lastFu == rhs.lastFu &&
        this->stopreq == rhs.stopreq &&
        this->bplcon0 == rhs.bplcon0 &&
        this->cnt == rhs.cnt;
    }

    bool operator!=(const DDFState &rhs) const
    {
        return !(*this == rhs);
    }
        
    template <class W>
    void operator<<(W& worker)
    {
        worker
        
        << bpv
        << bmapen
        << shw
        << rhw
        << bphstart
        << bphstop
        << bprun
        << lastFu
        << stopreq
        << bplcon0
        << cnt;
    }
};

#endif
