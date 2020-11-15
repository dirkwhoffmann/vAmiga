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
    data = new u8[ref.bsize]();
}

FSDataBlock::~FSDataBlock()
{
    delete [] data;
}


//
// Original File System (OFS)
//

OFSDataBlock::OFSDataBlock(FSVolume &ref, u32 nr) : FSDataBlock(ref, nr)
{
    data = new u8[ref.bsize]();
    
    set32(0, 8);    // Block type
}

void
OFSDataBlock::dump()
{    
}

bool
OFSDataBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    
    if (getDataBlockNr() < 1) {
        
        if (verbose) fprintf(stderr, "Invalid data block nr %d\n", getDataBlockNr());
        return false;
    }
    
    result &= assertNotNull(getFileHeaderRef(), verbose);
    result &= assertInRange(getFileHeaderRef(), verbose);
    result &= assertInRange(getNextDataBlockRef(), verbose);
    
    return result;
}

void
OFSDataBlock::updateChecksum()
{
    set32(5, 0);
    set32(5, checksum());
}

size_t
OFSDataBlock::addData(const u8 *buffer, size_t size)
{
    size_t headerSize = 24;
    size_t count = MIN(volume.bsize - headerSize, size);

    memcpy(data + headerSize, buffer, count);
    setDataBytesInBlock(count);
    
    return count;
}


//
// Fast File System (FFS)
//

FFSDataBlock::FFSDataBlock(FSVolume &ref, u32 nr) : FSDataBlock(ref, nr)
{
}

void
FFSDataBlock::dump()
{
}

bool
FFSDataBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    return result;
}

size_t
FFSDataBlock::addData(const u8 *buffer, size_t size)
{
    size_t count = MIN(volume.bsize, size);
    memcpy(data, buffer, count);
    return count;
}
