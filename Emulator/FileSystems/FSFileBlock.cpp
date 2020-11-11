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

void
FSFileBlock::dump()
{
    printf("\n");
    printf(" Block count: %d / %d\n", numDataBlockRefs(), maxDataBlockRefs());
    printf("       First: %d\n", firstDataBlock);
    printf("      Parent: %d\n", parent);
    printf("   Extension: %d\n", getNextExtensionBlockRef());
    printf(" Data blocks: ");
    for (int i = 0; i < numDataBlockRefs(); i++) printf("%d ", dataBlocks[i]);
}

bool
FSFileBlock::check(bool verbose)
{    
    bool result = FSBlock::check(verbose);
    
    assert(firstDataBlock == getFirstDataBlockRef());

    result &= assertNotNull(parent, verbose);
    result &= assertInRange(parent, verbose);
    result &= assertInRange(firstDataBlock, verbose);
    result &= assertInRange(getNextExtensionBlockRef(), verbose);

    for (int i = 0; i < maxDataBlockRefs(); i++) {
        result &= assertInRange(dataBlocks[i], verbose);
    }
    
    if (numDataBlockRefs() > 0 && firstDataBlock == 0) {
        if (verbose) fprintf(stderr, "Missing reference to first data block\n");
        return false;
    }
    
    if (numDataBlockRefs() < maxDataBlockRefs() && getNextExtensionBlockRef() != 0) {
        if (verbose) fprintf(stderr, "Unexpectedly found an extension block\n");
        return false;
    }
    
    return result;
}
