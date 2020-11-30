// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Utils.h"
#include "FSDevice.h"

FSPartition::FSPartition(u32 first, u32 last, u32 root)
{
    this->lowCyl = first;
    this->highCyl = last;
    this->rootBlock = root;

    assert(bmBlocks.size() == 0);
    assert(bmExtBlocks.size() == 0);
}

void
FSPartition::dump()
{
    msg("  First cylinder : %d\n", lowCyl);
    msg("   Last cylinder : %d\n", highCyl);
    msg("      Root block : %d\n", rootBlock);
    msg("   Bitmap blocks : ");
    for (auto& it : bmBlocks) { msg("%d ", it); }
    msg("\n");
    msg("Extension blocks : ");
    for (auto& it : bmExtBlocks) { msg("%d ", it); }
    msg("\n\n");
}

/*
u32
FSPartition::rootBlockRef()
{
    u32 highKey = numCyls() * dev.heads * dev.sectors - 1;
    u32 rootKey = (dev.reserved + highKey) / 2;
    
    return rootKey;
}

FSRootBlock *
FSPartition::rootBlockPtr()
{
    return dev.rootBlock(rootBlockRef());
}
*/
