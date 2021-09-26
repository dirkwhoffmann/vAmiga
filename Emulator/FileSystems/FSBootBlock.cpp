// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "BootBlockImage.h"
#include "FSBootBlock.h"
#include "FSDevice.h"
#include "FSPartition.h"

FSBootBlock::FSBootBlock(FSPartition &p, Block nr, FSBlockType t) : FSBlock(p, nr, t)
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

void
FSBootBlock::writeBootBlock(BootBlockId id, isize page)
{
    assert(page == 0 || page == 1);
    
    debug(FS_DEBUG, "writeBootBlock(%s, %zd)\n", BootBlockIdEnum::key(id), page);
    
    if (id != BB_NONE) {
        
        // Read boot block image from the database
        auto image = BootBlockImage(id);
        
        if (page == 0) {
            image.write(data + 4, 4, 511); // Write 508 bytes (skip header)
        } else {
            image.write(data, 512, 1023);  // Write 512 bytes
        }
    }
}
