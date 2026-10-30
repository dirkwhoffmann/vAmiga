// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "BasicTypes.h"
#include "utl/abilities/Streamable.h"

namespace vamiga {

static constexpr u16 SIG_NONE           = 0b0000000000;  ///< No signal
static constexpr u16 SIG_CON            = 0b0000000001;  ///< Continue
static constexpr u16 SIG_BMAPEN_CLR     = 0b0000000010;  ///< Clear the bitplane DMA enable flip-flop
static constexpr u16 SIG_BMAPEN_SET     = 0b0000000100;  ///< Set the bitplane DMA enable flip-flop
static constexpr u16 SIG_VFLOP_CLR      = 0b0000001000;  ///< Clear the vertical flip-flop
static constexpr u16 SIG_VFLOP_SET      = 0b0000010000;  ///< Set the vertical flip-flop
static constexpr u16 SIG_BPHSTART       = 000000100000;  ///< The DDFSTRT position has been reached
static constexpr u16 SIG_BPHSTOP        = 0b0001000000;  ///< The DDFSTOP position has been reached
static constexpr u16 SIG_SHW            = 0b0010000000;  ///< Start of the horizontal fetch window
static constexpr u16 SIG_RHW            = 0b0100000000;  ///< End of the horizontal fetch window
static constexpr u16 SIG_DONE           = 0b1000000000;  ///< The fetch sequence is complete

/// State of the data-fetch logic of the sequencer.
struct DDFState : utl::Streamable
{
    bool bpv = false;                   ///< Vertical bitplane DMA flip-flop (set inside the vertical DMA window)
    bool bmapen = false;                ///< Bitplane DMA enable (DMACON)
    bool shw = false;                   ///< DDF logic flip-flop (start of the horizontal fetch window)
    bool rhw = false;                   ///< DDF logic flip-flop (end of the horizontal fetch window)
    bool bphstart = false;              ///< Indicates that the DDFSTRT position has been reached
    bool bphstop = false;               ///< Indicates that the DDFSTOP position has been reached
    bool bprun = false;                 ///< Indicates that the bitplane fetch unit is running
    bool lastFu = false;                ///< Indicates that the last fetch unit is being processed
    bool stopreq = false;               ///< Indicates that the fetch unit has been asked to stop
    u16  bplcon0 = 0;                   ///< Value of BPLCON0
    u8   cnt = 0;                       ///< Fetch cycle counter within the fetch unit

    /// Compares two states.
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

    /// Compares two states.
    bool operator!=(const DDFState &rhs) const
    {
        return !(*this == rhs);
    }

    template <class W>
    /// Serializes the state (used for snapshots).
    W &operator<<(W& worker)
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

        return worker;
    }
};

}
