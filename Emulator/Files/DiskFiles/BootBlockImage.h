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
    const char *name;
    u16 signature[14];
    const u8 *image;
    size_t size;
    BootBlockType type;
}
BBRecord;

class BootBlockImage {

    // Image data
    u8 data[1024];
    
public:
    
    // Result of the data inspection
    BootBlockType type = BB_STANDARD;
    const char *name = "Unknown boot block";
    
    // Constructors
    BootBlockImage(const u8 *buffer);
    BootBlockImage(const char *name);
    BootBlockImage(long id);
    
    // Exports the image
    void write(u8 *buffer, size_t first = 0, size_t last = 0);
};

#endif
