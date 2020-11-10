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
    
    data = new u8[ref.bsize]();
    dataBytes = new u8[maxDataBytes]();
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
        for (int i = 0; i < numDataBytes; i++) p[numHeaderBytes + i] = dataBytes[i];
        
        // Checksum
        write32(p + 20, FSBlock::checksum(p));
        
    } else {
        
        // Data bytes
        for (int i = 0; i < numDataBytes; i++) p[i] = dataBytes[i];
    }
}

bool
FSDataBlock::append(const u8 *buffer, size_t size)
{
    size_t n = 0;
    
    // Get a reference to the file header block
    FSFileHeaderBlock *fhb = volume.fileHeaderBlock(fileHeaderBlock);
    if (fhb == nullptr) return false;
    
    // Seek the last data block
    FSDataBlock *dataBlock = this;
    while (dataBlock->next) dataBlock = volume.dataBlock(dataBlock->next);
    if (dataBlock == nullptr) return false;
        
    while (n < size) {
        
        // Fill the data block up
        size_t written = dataBlock->fillUp(buffer + n, size - n);
        n += written;
        fhb->fileSize += written;
        
        // Create a new data block if there are bytes remaining
        if (n < size) {
            
            FSDataBlock *newDataBlock = fhb->addDataBlockDeprecated();
            if (newDataBlock == nullptr) return false;
            
            // Connect the new block
            dataBlock->next = newDataBlock->nr;
            newDataBlock->fileHeaderBlock = fileHeaderBlock;
            newDataBlock->blockNumber = dataBlock->blockNumber + 1;
            dataBlock = newDataBlock;
        }
    }

    return true;
}

bool
FSDataBlock::append(const char *string)
{
    return append((u8 *)string, strlen(string));
}

size_t
FSDataBlock::addData(const u8 *buffer, size_t size)
{
    size_t count = MIN(maxDataBytes, size);
    
    // Copy bytes
    assert(numDataBytes == 0);
    for (int i = 0; i < count; i++) dataBytes[numDataBytes++] = buffer[i];

    return count;
}

size_t
FSDataBlock::fillUp(const u8 *buffer, size_t size)
{
    size_t freeSpace = maxDataBytes - numDataBytes;
    size_t count = MIN(size, freeSpace);

    // Copy bytes
    for (int i = 0; i < count; i++) dataBytes[numDataBytes++] = buffer[i];

    return count;
}

