// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSFileHeaderBlock.h"
#include "FSDevice.h"
#include "FSPartition.h"
#include <set>

FSFileHeaderBlock::FSFileHeaderBlock(FSPartition &p, Block nr, FSBlockType t) : FSBlock(p, nr, t)
{
    data = new u8[p.dev.bsize]();
   
    // Setup constant values
    
    set32(0, 2);                     // Type
    set32(1, nr);                    // Block pointer to itself
    setCreationDate(time(nullptr));  // Creation date
    set32(-1, (u32)-3);              // Sub type
}
