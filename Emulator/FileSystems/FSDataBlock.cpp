// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSDataBlock::FSDataBlock(FSVolume &ref, u32 nr, u32 cnt) : FSBlock(ref, nr)
{
    data = new u8[ref.bsize]();
    
    // Initialize header (OFS only)
    if (volume.isOFS()) {
    
        // Block type
        write32(data, 8);

        // Ordinal number (first block of a file = 1)
        write32(data + 8, cnt);
    }
}

FSDataBlock::~FSDataBlock()
{
    delete [] data;
}

void
FSDataBlock::dump()
{    
}

bool
FSDataBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    
    if (blockNumber < 1) {
        
        if (verbose) fprintf(stderr, "Block index %d is smaller than 1\n", blockNumber);
        return false;
    }

    /*
    result &= assertNotNull(fileHeaderBlock, verbose);
    result &= assertInRange(fileHeaderBlock, verbose);
    result &= assertInRange(blockNumber, verbose);
    result &= assertInRange(next, verbose);
    */
    
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
        // write32(p, 8);
        for (int i = 0; i < 4; i++) p[i] = data[i];

        // Reference to file header block
        // write32(p + 4, fileHeaderBlock);
        for (int i = 4; i < 8; i++) p[i] = data[i];

        // Number of the data block
        // write32(p + 8, blockNumber);
        for (int i = 8; i < 12; i++) p[i] = data[i];

        // Number of data bytes in this block
        // write32(p + 12, numDataBytes);
        for (int i = 12; i < 16; i++) p[i] = data[i];
        
        // Next data block
        // write32(p + 16, next);
        for (int i = 16; i < 20; i++) p[i] = data[i];

        // Data bytes
        for (int i = 24; i < volume.bsize; i++) p[i] = data[i];
        
        // Checksum
        write32(p + 20, FSBlock::checksum(p));
        
    } else {
        
        // Data bytes
        for (int i = 0; i < volume.bsize; i++) p[i] = data[i];
    }
}

/*
void
FSDataBlock::setDataBlockNr(u32 nr)
{
    assert(nr >= 1);

    // Write value into the OFS header
    if (volume.isOFS()) write32(data + 8, nr);
    blockNumber = nr;
}
*/

void
FSDataBlock::setFileHeaderRef(u32 ref)
{
    assert(volume.fileHeaderBlock(ref) != nullptr);
    
    // Write value into the OFS header
    if (volume.isOFS()) write32(data + 4, ref);
}

void
FSDataBlock::setNextDataBlockRef(u32 ref)
{
    assert(volume.dataBlock(ref) != nullptr);
    
    // Write value into the OFS header
    if (volume.isOFS()) write32(data + 16, ref);
    
    next = ref;
}

size_t
FSDataBlock::addData(const u8 *buffer, size_t size)
{
    size_t count = MIN(volume.dsize, size);
    size_t offset = volume.isOFS() ? 24 : 0;
    
    // Copy bytes
    for (int i = 0; i < count; i++) data[offset + i] = buffer[i];

    // Note number of written bytes in the header (OFS only)
    if (volume.isOFS()) write32(data + 12, count);
    
    return count;
}
