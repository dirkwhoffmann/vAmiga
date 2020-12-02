// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSDevice.h"

FSBootBlock::FSBootBlock(FSPartition &p, u32 nr, FSVolumeType type) : FSBlock(p, nr)
{
    data = new u8[bsize()]();
    
    // TODO: THIS CHECK ONLY WORKS CORRECTLY FOR THE FIRST PARTITION
    // SOLUTION: Add a new block of type FS_FIRST_BOOT_BLOCK 
    if (nr == 0) {
        data[0] = 'D';
        data[1] = 'O';
        data[2] = 'S';
        data[3] = (u8)type;
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
    if (strncmp((const char *)data, "DOS", 3)) return FS_NONE;
        
    // Only proceed if the DOS version number is valid
    if (data[3] > 7) return FS_NONE;
    
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
    msg("     Header : ");
    for (int i = 0; i < 8; i++) msg("%02X ", data[i]);
    msg("\n");
}

void
FSBootBlock::writeBootCode(FSBootCode bootCode, int page)
{
    assert(page == 0 || page == 1);
    
    u8 kick13[] = {
        
        0xc0, 0x20, 0x0f, 0x19, 0x00, 0x00, 0x03, 0x70, 0x43, 0xfa, 0x00, 0x18,
        0x4e, 0xae, 0xff, 0xa0, 0x4a, 0x80, 0x67, 0x0a, 0x20, 0x40, 0x20, 0x68,
        0x00, 0x16, 0x70, 0x00, 0x4e, 0x75, 0x70, 0xff, 0x60, 0xfa, 0x64, 0x6f,
        0x73, 0x2e, 0x6c, 0x69, 0x62, 0x72, 0x61, 0x72, 0x79
    };
    
    u8 kick20[] = {
        
        0xE3, 0x3D, 0x0E, 0x72, 0x00, 0x00, 0x03, 0x70, 0x43, 0xFA, 0x00, 0x3E,
        0x70, 0x25, 0x4E, 0xAE, 0xFD, 0xD8, 0x4A, 0x80, 0x67, 0x0C, 0x22, 0x40,
        0x08, 0xE9, 0x00, 0x06, 0x00, 0x22, 0x4E, 0xAE, 0xFE, 0x62, 0x43, 0xFA,
        0x00, 0x18, 0x4E, 0xAE, 0xFF, 0xA0, 0x4A, 0x80, 0x67, 0x0A, 0x20, 0x40,
        0x20, 0x68, 0x00, 0x16, 0x70, 0x00, 0x4E, 0x75, 0x70, 0xFF, 0x4E, 0x75,
        0x64, 0x6F, 0x73, 0x2E, 0x6C, 0x69, 0x62, 0x72, 0x61, 0x72, 0x79, 0x00,
        0x65, 0x78, 0x70, 0x61, 0x6E, 0x73, 0x69, 0x6F, 0x6E, 0x2E, 0x6C, 0x69,
        0x62, 0x72, 0x61, 0x72, 0x79
    };
    
    memset(data + 4, 0, bsize() - 4);
    
    if (page == 0) {
        
        switch (bootCode) {
                
            case FS_BOOTBLOCK_KICK_1_3:
                memcpy(data + 4, kick13, sizeof(kick13));
                break;
                
            case FS_BOOTBLOCK_KICK_2_0:
                memcpy(data + 4, kick20, sizeof(kick20));
                break;
                
            default:
                assert(false);
        }
    }
}
