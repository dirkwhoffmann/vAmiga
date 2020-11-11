// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSFileBlock::FSFileBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    dataBlocks = new u32[maxDataBlockRefs()]();
}

FSFileBlock::~FSFileBlock()
{
    delete [] dataBlocks;
}

bool
FSFileBlock::check(bool verbose)
{    
    bool result = FSBlock::check(verbose);
    
    result &= assertNotNull(getParentRef(), verbose);
    result &= assertInRange(getParentRef(), verbose);
    result &= assertInRange(getFirstDataBlockRef(), verbose);
    result &= assertInRange(getNextExtensionBlockRef(), verbose);

    for (int i = 0; i < maxDataBlockRefs(); i++) {
        result &= assertInRange(dataBlocks[i], verbose);
    }
    
    if (numDataBlockRefs() > 0 && getFirstDataBlockRef() == 0) {
        if (verbose) fprintf(stderr, "Missing reference to first data block\n");
        return false;
    }
    
    if (numDataBlockRefs() < maxDataBlockRefs() && getNextExtensionBlockRef() != 0) {
        if (verbose) fprintf(stderr, "Unexpectedly found an extension block\n");
        return false;
    }
    
    return result;
}
