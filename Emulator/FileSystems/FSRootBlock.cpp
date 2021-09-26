// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSRootBlock.h"
#include "FSDevice.h"
#include "FSPartition.h"

FSRootBlock::FSRootBlock(FSPartition &p, Block nr, FSBlockType t) : FSBlock(p, nr, t)
{
    data = new u8[bsize()]();
    
    assert(hashTableSize() == 72);
    
    set32(0, 2);                         // Type
    set32(3, (u32)hashTableSize());      // Hash table size
    set32(-50, 0xFFFFFFFF);              // Bitmap validity
    setCreationDate(time(nullptr));      // Creation date
    setModificationDate(time(nullptr));  // Modification date
    set32(-1, 1);                        // Sub type    
}
