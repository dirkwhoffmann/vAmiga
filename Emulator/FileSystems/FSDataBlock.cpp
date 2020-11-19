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

FSError
OFSDataBlock::check(u32 pos)
{
    // Align pos to the long word raster
    pos &= ~0b11;

    // Translate 'pos' to a long word index
    i32 word = pos / 4;

    u32 value = get32(word);
    
    switch (word) {
        case 0:
            return value == 8 ? FS_OK : FS_BLOCK_TYPE_ID_MISMATCH;
        case 1:
            return volume.fileHeaderBlock(value) ? FS_OK : FS_BLOCK_MISSING_FILEHEADER_REF;
        case 2:
            return value > 0 ? FS_OK : FS_BLOCK_MISSING_DATABLOCK_NUMBER;
        case 3:
            return value <= volume.dsize ? FS_OK : FS_BLOCK_VALUE_TOO_LARGE;
        case 4:
            if (value && !volume.dataBlock(value)) return FS_BLOCK_NO_DATABLOCK_REF;
        case 5:
            return value == checksum() ? FS_OK : FS_BLOCK_CHECKSUM_ERROR;
        default:
            break;
    }
        
    return FS_OK;
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
