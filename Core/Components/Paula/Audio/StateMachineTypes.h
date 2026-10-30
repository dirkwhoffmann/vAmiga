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

namespace vamiga {

//
// Structures
//

/// State of an audio state machine.
typedef struct
{
    isize state;                            ///< State of the audio state machine
    bool dma;                               ///< Audio DMA is enabled for this channel
    u16 audlenLatch;                        ///< Length latch (AUDxLEN)
    u16 audlen;                             ///< Remaining length
    u16 audperLatch;                        ///< Period latch (AUDxPER)
    i32 audper;                             ///< Remaining period
    u16 audvolLatch;                        ///< Volume latch (AUDxVOL)
    u16 audvol;                             ///< Current volume
    u16 auddat;                             ///< Data register (AUDxDAT)
}
StateMachineInfo;

}
