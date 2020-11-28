// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSBitmapBlock::FSBitmapBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    data = new u8[ref.bsize]();
}

FSBitmapBlock::~FSBitmapBlock()
{
    delete [] data;
}

FSItemType
FSBitmapBlock::itemType(u32 pos)
{
    return pos < 4 ? FSI_CHECKSUM : FSI_BITMAP;
}

FSError
FSBitmapBlock::check(u32 byte, u8 *expected, bool strict)
{
    i32 word = byte / 4;
    u32 value = get32(word);
    
    if (word == 0) EXPECT_CHECKSUM;
    
    return FS_OK;
}

void
FSBitmapBlock::dump()
{
    u32 count = 0;
    for (u32 i = 1; i < volume.bsize / 4; i++) {
        if (u32 value = get32(i)) {
            for (int j = 0; j < 8; j++) {
                if (GET_BIT(value, j)) count++;
            }
        }
    }
    printf("   Allocated: %d blocks\n", count);
}
