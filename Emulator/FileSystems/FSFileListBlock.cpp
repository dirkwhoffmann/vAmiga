// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSFileListBlock::FSFileListBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    data = new u8[ref.bsize]();
}

FSFileListBlock::~FSFileListBlock()
{
    delete [] data;
}

void
FSFileListBlock::dump()
{
    printf(" Block count : %d / %d\n", numDataBlockRefs(), maxDataBlockRefs());
    printf("       First : %d\n", getFirstDataBlockRef());
    printf("Header block : %d\n", getFileHeaderRef());
    printf("   Extension : %d\n", getNextExtensionBlockRef());
    printf(" Data blocks : ");
    for (int i = 0; i < numDataBlockRefs(); i++) printf("%d ", getDataBlockRef(i));
    printf("\n");
}

bool
FSFileListBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    
    result &= assertNotNull(getFileHeaderRef(), verbose);
    result &= assertInRange(getFileHeaderRef(), verbose);
    result &= assertInRange(getFirstDataBlockRef(), verbose);
    result &= assertInRange(getNextExtensionBlockRef(), verbose);

    for (int i = 0; i < maxDataBlockRefs(); i++) {
        result &= assertInRange(getDataBlockRef(i), verbose);
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

void
FSFileListBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(p);
    assert(volume.bsize == bsize);

    memcpy(p, data, bsize);
    
    // Type
    write32(p, 16);
    
    // Block pointer to itself
    write32(p + 4, nr);
    
    // Number of data block references
    write32(p + 8, numDataBlockRefs());
    
    // First data block
    // write32(p + 16, firstDataBlock);
    
    // Data block list
    u8 *end = p + bsize - 51 * 4;
    for (int i = 0; i < numDataBlockRefs(); i++) {
        write32(end - 4 * i, getDataBlockRef(i));
    }
    
    // Block pointer to parent directory
    // write32(p + bsize - 3 * 4, parent);
    
    // Block pointer to first extension block
    // write32(p + bsize - 2 * 4, nextTableBlock);
    
    // Subtype
    write32(p + bsize - 1 * 4, (u32)-3);
    
    // Checksum
    write32(p + 20, FSBlock::checksum(p));
}

bool
FSFileListBlock::addDataBlockRef(u32 first, u32 ref)
{
    if (numDataBlockRefs() < maxDataBlockRefs()) {

        setFirstDataBlockRef(first);
        setDataBlockRef(numDataBlockRefs(), ref);
        // dataBlocks[numDataBlockRefs()] = ref;
        incDataBlockRefs();
        return true;
    }

    return false;
}
