// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSFileListBlock.h"
#include "FSDevice.h"
#include "FSPartition.h"

FSFileListBlock::FSFileListBlock(FSPartition &p, Block nr, FSBlockType t) : FSBlock(p, nr, t)
{
    data = new u8[bsize()]();

    set32(0, 16);                         // Type
    set32(1, nr);                         // Block pointer to itself
    set32(-1, (u32)-3);                   // Sub type
}

FSFileListBlock::~FSFileListBlock()
{
    delete [] data;
}

/*
bool
FSFileListBlock::addDataBlockRef(Block first, Block ref)
{
    // The caller has to ensure that this block contains free slots
    if (getNumDataBlockRefs() < getMaxDataBlockRefs()) {

        setFirstDataBlockRef(first);
        setDataBlockRef(getNumDataBlockRefs(), ref);
        incNumDataBlockRefs();
        return true;
    }

    return false;
}
*/
