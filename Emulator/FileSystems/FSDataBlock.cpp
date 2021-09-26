// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSDataBlock.h"
#include "FSDevice.h"
#include "FSPartition.h"

FSDataBlock::FSDataBlock(FSPartition &p, u32 nr, FSBlockType t) : FSBlock(p, nr, t)
{
    data = new u8[p.dev.bsize]();
}

FSDataBlock::~FSDataBlock()
{
    delete [] data;
}


//
// Original File System (OFS)
//

OFSDataBlock::OFSDataBlock(FSPartition &p, u32 nr, FSBlockType t) : FSDataBlock(p, nr, t)
{
    data = new u8[bsize()]();
    
    set32(0, 8); // Block type
}

/*
isize
OFSDataBlock::writeData(FILE *file, isize size)
{
    assert(file != nullptr);
    
    isize count = std::min(dsize(), size);
    for (isize i = 0; i < count; i++) fputc(data[i + 24], file);
    return count;
}
*/

isize
OFSDataBlock::addData(const u8 *buffer, isize size)
{
    isize count = std::min(bsize() - 24, size);

    std::memcpy(data + 24, buffer, count);
    setDataBytesInBlock((u32)count);
    
    return count;
}

//
// Fast File System (FFS)
//

FFSDataBlock::FFSDataBlock(FSPartition &p, u32 nr, FSBlockType t) : FSDataBlock(p, nr, t) { }

/*
isize
FFSDataBlock::writeData(FILE *file, isize size)
{
    assert(file != nullptr);
    
    isize count = std::min(dsize(), size);
    for (isize i = 0; i < count; i++) fputc(data[i], file);
    return count;
}
*/

isize
FFSDataBlock::addData(const u8 *buffer, isize size)
{
    isize count = std::min(bsize(), size);
    std::memcpy(data, buffer, count);
    return count;
}
