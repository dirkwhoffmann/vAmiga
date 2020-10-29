// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSDataBlock::FSDataBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    maxDataBytes = volume.isOFS() ? 488 : 512;
    data = new u8[maxDataBytes]();
}

FSDataBlock::~FSDataBlock()
{
    delete [] data;
}

void
FSDataBlock::dump()
{    
    printf("   Block index: %d\n", blockNumber);
    printf("  Header block: %d\n", fileHeaderBlock);
    printf(" Byte capacity: %d\n", maxDataBytes);
    printf("    Bytes used: %d\n", numDataBytes);
    printf("          Next: %d\n", next);
}

bool
FSDataBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    
    if (blockNumber < 1) {
        
        if (verbose) fprintf(stderr, "Block index %d is smaller than 1\n", blockNumber);
        return false;
    }
    
    result &= assertNotNull(fileHeaderBlock, verbose);
    result &= assertInRange(fileHeaderBlock, verbose);
    result &= assertInRange(blockNumber, verbose);
    result &= assertInRange(next, verbose);
    
    return result;
}

void
FSDataBlock::write(u8 *p)
{
    // Start from scratch
    memset(p, 0, 512);

    // TODO
}

bool
FSDataBlock::append(u8 byte)
{
    // Get a reference to the file header block
    FSFileHeaderBlock *fhb = volume.fileHeaderBlock(fileHeaderBlock);
    if (fhb == nullptr) return false;
    
    // If there is space for another byte, add it
    if (numDataBytes < maxDataBytes) {
        
        data[numDataBytes] = byte;
        numDataBytes++;
        return true;
    }
    
    // Otherwise, create a new data block
    FSDataBlock *block = volume.newDataBlock();
    if (block == nullptr) return false;
    
    // Add a reference of this block to the file header block
    if (!fhb->addDataBlockRef(block->nr)) {
        volume.deallocateBlock(block->nr);
        return false;
    }
    // Connect the new block
    block->next = block->nr;
    block->fileHeaderBlock = fileHeaderBlock;
    block->blockNumber = blockNumber + 1;
    
    return true;
}

bool
FSDataBlock::append(u8 *buffer, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (!append(buffer[i])) return false;
    }
    
    return true;
}
