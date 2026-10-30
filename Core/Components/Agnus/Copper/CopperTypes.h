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

/// State of the Copper (registers and program counter).
typedef struct
{
    isize copList;                          ///< Number of the active Copper list (1 or 2)
    u32   copList1Start;                    ///< Start address of Copper list 1
    u32   copList1End;                      ///< End address of Copper list 1
    u32   copList2Start;                    ///< Start address of Copper list 2
    u32   copList2End;                      ///< End address of Copper list 2
    bool  active;                           ///< The Copper is running
    bool  cdang;                            ///< Copper danger bit (COPCON): the Copper may access the Blitter registers
    u32   coppc0;                           ///< Program counter (start of the current instruction)
    u32   cop1lc;                           ///< Location register of Copper list 1 (COP1LC)
    u32   cop2lc;                           ///< Location register of Copper list 2 (COP2LC)
    u16   cop1ins;                          ///< First instruction word
    u16   cop2ins;                          ///< Second instruction word
}
CopperInfo;

}
