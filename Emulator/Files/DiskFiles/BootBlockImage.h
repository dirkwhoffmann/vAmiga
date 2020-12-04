// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _BOOT_BLOCK_IMAGE_H
#define _BOOT_BLOCK_IMAGE_H

#include "FileTypes.h"

typedef struct
{
    BootBlockIdentifier id;
    BootBlockType type;
    const char *name;
    u16 signature[14];
}
BBRecord;

class BootBlockImage {

    // Image data
    u8 data[1024];
    
public:
    
    // Result of the data inspection
    BootBlockIdentifier id = BB_UNKNOWN;
    BootBlockType type = BB_STANDARD;
    const char *name = "Unknown boot block";
    
    // Constructors
    BootBlockImage(const unsigned char *buf);
    BootBlockImage(BootBlockIdentifier id);
    
    // Exports the image
    void write(u8 *buffer, size_t first = 0, size_t last = 0);

private:
    
    // Searches the data base
    void detect();
};

#endif
