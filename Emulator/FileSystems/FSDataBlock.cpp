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
    
    set32(0, 8); // Block type
}

void
OFSDataBlock::dump()
{
    printf("File header block : %d\n", getFileHeaderRef());
    printf("     Chain number : %d\n", getDataBlockNr());
    printf("       Data bytes : %d\n", getDataBytesInBlock());
    printf("  Next data block : %d\n", getNextDataBlockRef());
    printf("\n");
}

FSItemType
OFSDataBlock::itemType(u32 pos)
{
    if (pos < 24) {
        
        i32 word = pos / 4;
        
        switch (word) {
                
            case 0: return FSI_TYPE_ID;
            case 1: return FSI_FILEHEADER_REF;
            case 2: return FSI_DATA_BLOCK_NUMBER;
            case 3: return FSI_DATA_COUNT;
            case 4: return FSI_NEXT_DATA_BLOCK_REF;
            case 5: return FSI_CHECKSUM;
        }
    }
    
    return FSI_DATA;
}

FSError
OFSDataBlock::check(u32 byte)
{
    if (byte < 24) {
        
        i32 word = byte / 4;
        u32 value = get32(word);
        
        switch (word) {
                
            case 0: EXPECT_00000008(value, byte % 4); break;
            case 1: EXPECT_FILEHEADER_REF(value); break;
            case 2: EXPECT_DATABLOCK_NUMBER(value); break;
            case 3: EXPECT_RANGE(value, 0, volume.dsize); break;
            case 4: EXPECT_OPTIONAL_DATABLOCK_REF(value); break;
            case 5: EXPECT_CHECKSUM(value); break;
        }
    }
    
    return FS_OK;
}

size_t
OFSDataBlock::addData(const u8 *buffer, size_t size)
{
    size_t count = MIN(volume.bsize - headerSize(), size);

    memcpy(data + headerSize(), buffer, count);
    setDataBytesInBlock(count);
    
    return count;
}


//
// Fast File System (FFS)
//

FFSDataBlock::FFSDataBlock(FSVolume &ref, u32 nr) : FSDataBlock(ref, nr) { }

void
FFSDataBlock::dump()
{
}

FSItemType
FFSDataBlock::itemType(u32 pos)
{
    return FSI_DATA;
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
