// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSDevice.h"
#include "BootBlockImage.h"

FSBootBlock::FSBootBlock(FSPartition &p, u32 nr) : FSBlock(p, nr)
{
    data = new u8[bsize()]();
    
    if (nr == p.firstBlock && p.dos != FS_NODOS) {
        data[0] = 'D';
        data[1] = 'O';
        data[2] = 'S';
        data[3] = (u8)p.dos;
    }
}

FSBootBlock::~FSBootBlock()
{
    delete [] data;
}


FSVolumeType
FSBootBlock::dos()
{
    // Only proceed if the header begins with 'DOS'
    if (strncmp((const char *)data, "DOS", 3)) return FS_NODOS;
        
    // Only proceed if the DOS version number is valid
    if (data[3] > 7) return FS_NODOS;
    
    return (FSVolumeType)data[3];
}

FSItemType
FSBootBlock::itemType(u32 byte)
{
    if (byte <= 3 && nr == 0) {
        
        switch(byte) {
            case 0:
            case 1:
            case 2: return FSI_DOS_HEADER;
            case 3: return FSI_DOS_VERSION;
        }
    }
    
    return FSI_BOOTCODE;
}

FSError
FSBootBlock::check(u32 byte, u8 *expected, bool strict)
{
    if (byte <= 3 && nr == 0) {
        
        u8 value = data[byte];
        
        switch(byte) {
            case 0: EXPECT_BYTE('D'); break;
            case 1: EXPECT_BYTE('O'); break;
            case 2: EXPECT_BYTE('S'); break;
            case 3: EXPECT_DOS_REVISION; break;
        }
    }
    
    return FS_OK;
}

void
FSBootBlock::dump()
{
    msg("       Header : ");
    for (int i = 0; i < 8; i++) msg("%02X ", data[i]);
    msg("\n");
}

void
FSBootBlock::writeBootBlock(long bootBlockID, int page)
{
    assert(page == 0 || page == 1);
    
    debug(FS_DEBUG, "writeBootBlock(id: %d, page: %d)\n", bootBlockID, page);
    
    // Read boot block image from the database
    BootBlockImage image = BootBlockImage(bootBlockID);
    
    if (page == 0) {
        image.write(data + 4, 4, 511); // Write 508 bytes (skip header)
    } else {
        image.write(data, 512, 1023);  // Write 512 bytes
    }
}
