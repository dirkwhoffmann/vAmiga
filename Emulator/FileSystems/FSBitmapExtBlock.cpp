// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSBitmapExtBlock::FSBitmapExtBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    data = new u8[ref.bsize]();
}

FSBitmapExtBlock::~FSBitmapExtBlock()
{
    delete [] data;
}

FSItemType
FSBitmapExtBlock::itemType(u32 pos)
{
    return pos < (volume.bsize - 4) ? FSI_BITMAP : FSI_BITMAP_EXT_BLOCK_REF;
}

FSError
FSBitmapExtBlock::check(u32 byte, u8 *expected, bool strict)
{
    i32 word = byte / 4;
    u32 value = get32(word);
    
    if (word == (i32)volume.bsize - 4) EXPECT_BITMAP_EXT_REF;
    
    return FS_OK;
}

void
FSBitmapExtBlock::dump()
{
}
