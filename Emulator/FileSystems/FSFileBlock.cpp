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
    maxDataBlocks = 72;
    dataBlocks = new u32[maxDataBlocks]();
}

FSFileBlock::~FSFileBlock()
{
    delete [] dataBlocks;
}

void
FSFileBlock::dump()
{
    printf("\n");
    printf(" Block count: %d / %d\n", numDataBlocks, maxDataBlocks);
    printf("       First: %d\n", firstDataBlock);
    printf("      Parent: %d\n", parent);
    printf("   Extension: %d\n", nextTableBlock);
    printf(" Data blocks: ");
    for (int i = 0; i < numDataBlocks; i++) printf("%d ", dataBlocks[i]);
}

bool
FSFileBlock::check(bool verbose)
{    
    bool result = FSBlock::check(verbose);
    
    result &= assertNotNull(parent, verbose);
    result &= assertInRange(parent, verbose);
    result &= assertInRange(firstDataBlock, verbose);
    result &= assertInRange(nextTableBlock, verbose);

    for (int i = 0; i < maxDataBlocks; i++) {
        result &= assertInRange(dataBlocks[i], verbose);
    }
    
    if (numDataBlocks > 0 && firstDataBlock == 0) {
        if (verbose) fprintf(stderr, "Missing reference to first data block\n");
        return false;
    }
    
    if (numDataBlocks < maxDataBlocks && nextTableBlock != 0) {
        if (verbose) fprintf(stderr, "Unexpectedly found an extension block\n");
        return false;
    }
    
    return result;
}

FSDataBlock *
FSFileBlock::addDataBlockDeprecated()
{
    // Create a new data block
    FSDataBlock *block = volume.newDataBlock();
    if (block == nullptr) return nullptr;
    
    // Add a reference to this block
    if (!addDataBlockRef(block->nr)) {
        volume.deallocateBlock(block->nr);
        return nullptr;
    }

    return block;
}

bool
FSFileBlock::addDataBlockRef(u32 ref)
{
    printf("FSFileBlock::addDataBlockRef(%d)\n", ref);

    // Otherwise, try to add the reference in this block
    if (numDataBlocks < maxDataBlocks) {

        printf("Adding block ref %d at %d\n", ref, numDataBlocks);

        dataBlocks[numDataBlocks++] = ref;
        return true;
    }

    // If there is an extension block, add it there
    FSFileBlock *extension = volume.fileListBlock(nextTableBlock);
    if (extension) return extension->addDataBlockRef(ref);
    
    assert(false);
    // TODO: RETURN false HERE. THE NEW CODE WILL CREATE THE NECESSARY AMOUNT
    // TODO: OF NEW EXTENSION BLOCKS BEFORE CALLING THIS FUNCTION
    
    // If this block is full, create a new FileListBlock
    u32 newRef = volume.addFileListBlock(parent, nr);
    FSFileListBlock *block = volume.fileListBlock(newRef);
    if (block == nullptr) return false;

    // Connect the block
    block->firstDataBlock = firstDataBlock;
    
    // Add the reference to the new block
    return block->addDataBlockRef(ref);
}

void
FSFileBlock::deleteDataBlockRefs()
{
    numDataBlocks = 0;
    for (int i = 0; i < maxDataBlocks; i++) dataBlocks[i] = 0;
}
