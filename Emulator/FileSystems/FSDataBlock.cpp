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
    
    /*
    if (blockNumber < 1) {
        
        if (verbose) fprintf(stderr, "Block index %d is smaller than 1\n", blockNumber);
        return false;
    }
    */
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

    // Write block data
    for (int i = 0; i < bsize; i++) p[i] = data[i];
    
    // Compute checksum
    if (volume.isOFS()) write32(p + 20, FSBlock::checksum(p));
}

void
FSDataBlock::setFileHeaderRef(u32 ref)
{
    assert(volume.fileHeaderBlock(ref) != nullptr);
    
    // Write value into the OFS header
    if (volume.isOFS()) write32(data + 4, ref);
}


u32
FSDataBlock::getNextDataBlockRef()
{
    return volume.isOFS() ? read32(data + 16) : 0;
}

void
FSDataBlock::setNextDataBlockRef(u32 ref)
{
    if (volume.isOFS()) write32(data + 16, ref);
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
