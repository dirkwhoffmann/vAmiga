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
    printf(" Data blocks: ");
    for (int i = 0; i < numDataBlocks; i++) printf("%d ", dataBlocks[i]);
    printf("\n");
    printf("       First: %d\n", firstDataBlock);
    printf("      Parent: %d\n", parent);
    printf("        Next: %d\n", nextTableBlock);
}

bool
FSFileBlock::check(bool verbose)
{
    printf("FSRefTableBlock::check(%d)", verbose);
    
    bool result = FSBlock::check(verbose);
    
    result &= assertNotNull(parent, verbose);
    result &= assertInRange(parent, verbose);
    result &= assertInRange(firstDataBlock, verbose);
    result &= assertInRange(nextTableBlock, verbose);

    for (int i = 0; i < maxDataBlocks; i++) {
        result &= assertInRange(dataBlocks[i], verbose);
    }
    
    return result;
}

FSDataBlock *
FSFileBlock::addDataBlock()
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
    // If there is space for another reference, add it
    if (numDataBlocks < maxDataBlocks) {

        dataBlocks[numDataBlocks] = ref;
        numDataBlocks++;
        return true;
    }
    
    // Otherwise, create a new FileListBlock
    FSFileListBlock *block = volume.newFileListBlock();
    if (block == nullptr) return false;
    
    // Connect the new block
    block->firstDataBlock = firstDataBlock;
    block->parent = parent;
    
    // Add the reference to the new block
    return block->addDataBlockRef(ref);
}

void
FSFileBlock::deleteDataBlockRefs()
{
    numDataBlocks = 0;
    for (int i = 0; i < maxDataBlocks; i++) dataBlocks[i] = 0;
}
