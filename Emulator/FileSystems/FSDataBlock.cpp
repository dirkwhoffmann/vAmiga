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
    numHeaderBytes = volume.isOFS() ? 24 : 0;
    maxDataBytes = volume.bsize - numHeaderBytes;
    
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
FSDataBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(p);
    assert(volume.bsize == bsize);

    // Start from scratch
    memset(p, 0, bsize);
        
    if (numHeaderBytes) {
        
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
        for (int i = 0; i < numDataBytes; i++) p[numHeaderBytes + i] = data[i];
        
        // Checksum
        write32(p + 20, FSBlock::checksum(p));
        
    } else {
        
        // Data bytes
        for (int i = 0; i < numDataBytes; i++) p[i] = data[i];
    }
}

bool
FSDataBlock::append(u8 byte)
{
    // Get a reference to the file header block
    FSFileHeaderBlock *fhb = volume.fileHeaderBlock(fileHeaderBlock);
    if (fhb == nullptr) return false;
    
    // Append at the end
    if (next) {
        FSBlock *nextBlock = volume.block(next);
        if (nextBlock) {
            assert(numDataBytes == maxDataBytes);
            return nextBlock->append(byte);
        }
    }
    
    // If there is space for another byte, add it
    if (numDataBytes < maxDataBytes) {
        
        printf("Adding byte to block %d (%d, %d)\n", nr, numDataBytes, maxDataBytes);
        
        data[numDataBytes] = byte;
        numDataBytes++;
        fhb->fileSize++;
        return true;
    }

    printf("create a new data block (%d, %d, %d)\n", nr, numDataBytes, maxDataBytes);

    // Otherwise, create a new data block
    FSDataBlock *block = fhb->addDataBlock();
    if (block == nullptr) return false;
    
    // Connect the new block
    next = block->nr;
    block->fileHeaderBlock = fileHeaderBlock;
    block->blockNumber = blockNumber + 1;
    
    return true;
}

bool
FSDataBlock::append(const u8 *buffer, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (!append(buffer[i])) return false;
    }
    
    return true;
}

bool
FSDataBlock::append(const char *buffer, size_t size)
{
    return append((u8 *)buffer, size);
}
