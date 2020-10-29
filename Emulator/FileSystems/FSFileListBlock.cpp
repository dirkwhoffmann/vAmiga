// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

void
FSFileListBlock::dump()
{
    FSFileBlock::dump();
}

bool
FSFileListBlock::check(bool verbose)
{
    printf("FSFileListBlock::check(%d)", verbose);

    return FSFileBlock::check(verbose);
}

void
FSFileListBlock::write(u8 *p)
{
    // Start from scratch
    memset(p, 0, 512);

    // TODO
}

FSFileListBlock::FSFileListBlock(FSVolume &ref, u32 nr) : FSFileBlock(ref, nr)
{
    
}
