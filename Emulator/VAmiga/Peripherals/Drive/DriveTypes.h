// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MediaFileTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class DiskFlags
{
    PROTECTED  = 1,
    MODIFIED   = 2
};

struct DiskFlagsEnum : Reflection<DiskFlagsEnum, DiskFlags>
{
    static constexpr long minVal = 1;
    static constexpr long maxVal = long(DiskFlags::MODIFIED);
    
    static const char *_key(DiskFlags value)
    {
        switch (value) {
                
            case DiskFlags::PROTECTED:    return "PROTECTED";
            case DiskFlags::MODIFIED:     return "MODIFIED";
        }
        return "???";
    }
    static const char *help(DiskFlags value)
    {
        switch (value) {
                
            case DiskFlags::PROTECTED:    return "Write protected disk";
            case DiskFlags::MODIFIED:     return "Modified disk contents";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    Cylinder cylinder;
    Head head;
    isize offset;
}
DriveHead;

}
