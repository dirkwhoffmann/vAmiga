// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FileTypes.hpp"
#include <stddef.h>

namespace va {

#include "BootBlockImage.h"

struct BootBlockTypeEnum : util::Reflection<BootBlockTypeEnum, BootBlockType> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < BB_COUNT;
    }
    
    static const char *prefix() { return "BB"; }
    static const char *key(BootBlockType value)
    {
        switch (value) {
                
            case BB_STANDARD:  return "STANDARD";
            case BB_VIRUS:     return "VIRUS";
            case BB_CUSTOM:    return "CUSTOM";
            case BB_COUNT:     return "???";
        }
        return "???";
    }
};

struct BootBlockIdEnum : util::Reflection<BootBlockIdEnum, BootBlockId> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < BB_COUNT;
    }
    
    static const char *prefix() { return "BB"; }
    static const char *key(BootBlockId value)
    {
        switch (value) {
                
            case BB_NONE:         return "NONE";
            case BB_AMIGADOS_13:  return "AMIGADOS_13";
            case BB_AMIGADOS_20:  return "AMIGADOS_20";
            case BB_SCA:          return "SCA";
            case BB_BYTE_BANDIT:  return "BYTE_BANDIT";
        }
        return "???";
    }
};

typedef struct
{
    const char *name;
    u16 signature[14];
    const u8 *image;
    isize size;
    BootBlockType type;
}
BBRecord;

class BootBlockImage {

    // Image data
    u8 data[1024];
    
public:
    
    // Result of the data inspection
    BootBlockType type = BB_CUSTOM;
    const char *name = "Custom boot block";
    
    // Constructors
    BootBlockImage(const u8 *buffer);
    BootBlockImage(const char *name);
    BootBlockImage(BootBlockId id);
    
    // Exports the image
    void write(u8 *buffer, isize first = 0, isize last = 0);
};

}
