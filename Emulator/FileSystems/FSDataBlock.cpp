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
    // numHeaderBytes = volume.isOFS() ? 24 : 0;
    // maxDataBytes = volume.bsize - numHeaderBytes;
    // assert(maxDataBytes == volume.dsize);
    
    data = new u8[ref.bsize]();
    dataBytes = new u8[volume.dsize]();
}

FSDataBlock::~FSDataBlock()
{
    delete [] data;
    delete [] dataBytes;
}

void
FSDataBlock::dump()
{    
    printf("   Block index: %d\n", blockNumber);
    printf("  Header block: %d\n", fileHeaderBlock);
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
FSDataBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(p);
    assert(volume.bsize == bsize);

    // Start from scratch
    memset(p, 0, bsize);
        
    if (volume.isOFS()) {
        
        // Type
        write32(p, 8);
        
        // Reference to file header block
        write32(p + 4, fileHeaderBlock);
        
        // Number of the data block
        write32(p + 8, blockNumber);
        
        // Number of data bytes in this block
        write32(p + 12, numDataBytes);
        
        // Number of data bytes in this block
        write32(p + 16, next);

        // Data bytes
        for (int i = 0; i < numDataBytes; i++) p[24 + i] = dataBytes[i];
        
        // Checksum
        write32(p + 20, FSBlock::checksum(p));
        
    } else {
        
        // Data bytes
        for (int i = 0; i < numDataBytes; i++) p[i] = dataBytes[i];
    }
}

size_t
FSDataBlock::addData(const u8 *buffer, size_t size)
{
    size_t count = MIN(volume.dsize, size);
    
    // Copy bytes
    assert(numDataBytes == 0);
    for (int i = 0; i < count; i++) dataBytes[numDataBytes++] = buffer[i];

    return count;
}
