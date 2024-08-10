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
#include "Reflection.h"
#include "FSTypes.h"
#include "BootBlockImageTypes.h"

//
// Enumerations
//

enum_long(DIAMETER)
{
    INCH_35,
    INCH_525
};
typedef DIAMETER Diameter;

#ifdef __cplusplus
struct DiameterEnum : vamiga::util::Reflection<DiameterEnum, Diameter>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = INCH_525;

    static const char *prefix() { return ""; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case INCH_35:     return "INCH_35";
            case INCH_525:    return "INCH_525";
        }
        return "???";
    }
};
#endif

enum_long(DENSITY)
{
    DENSITY_SD,
    DENSITY_DD,
    DENSITY_HD
};
typedef DENSITY Density;

#ifdef __cplusplus
struct DensityEnum : vamiga::util::Reflection<DensityEnum, Density>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = DENSITY_HD;
    
    static const char *prefix() { return "DENSITY"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case DENSITY_SD:     return "SD";
            case DENSITY_DD:     return "DD";
            case DENSITY_HD:     return "HD";
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
    Diameter diameter;
    Density density;
    isize sides;
    isize cylinders;
}
FloppyDiskDescriptor;

typedef struct
{
    FSVolumeType dos;
    Diameter diameter;
    Density density;
    BootBlockType bootBlockType;
    const char *bootBlockName;
    bool hasVirus;
}
FloppyDiskInfo;
