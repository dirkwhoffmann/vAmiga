// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "MediaFileTypes.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(FLAG_DISK)
{
    FLAG_PROTECTED  = 1,
    FLAG_MODIFIED   = 2
};
typedef FLAG_DISK DiskFlags;

#ifdef __cplusplus
struct DiskFlagsEnum : vamiga::util::Reflection<DiskFlagsEnum, DiskFlags>
{
    static constexpr long minVal = 1;
    static constexpr long maxVal = FLAG_MODIFIED;

    static const char *prefix() { return "FLAG"; }
    static const char *_key(long value)
    {
        switch (value) {

            case FLAG_PROTECTED:    return "PROTECTED";
            case FLAG_MODIFIED:     return "MODIFIED";
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
    Cylinder cylinder;
    Head head;
    isize offset;
}
DriveHead;
