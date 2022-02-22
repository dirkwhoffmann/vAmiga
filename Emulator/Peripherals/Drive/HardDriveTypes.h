// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(HDR_TYPE)
{
    HDR_GENERIC
};
typedef HDR_TYPE HardDriveType;

#ifdef __cplusplus
struct HardDriveTypeEnum : util::Reflection<HardDriveTypeEnum, HardDriveType>
{
    static long minVal() { return 0; }
    static long maxVal() { return HDR_GENERIC; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }
    
    static const char *prefix() { return "HDR"; }
    static const char *key(HardDriveType value)
    {
        switch (value) {
                
            case HDR_GENERIC:   return "GENERIC";
        }
        return "???";
    }
};
#endif


//
// Structures
//

typedef struct
{
    Side side;
    Cylinder cylinder;
    Sector sector;
}
HardDriveHead;

typedef struct
{
    HardDriveType type;
    bool connected;
}
HardDriveConfig;

typedef struct
{
    bool attached;
    bool modified;
    struct { isize c; isize h; isize s; } head;
}
HardDriveInfo;
