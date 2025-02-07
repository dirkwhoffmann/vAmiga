// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "BootBlockImageTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class Diameter : long
{
    INCH_35,
    INCH_525
};

struct DiameterEnum : Reflection<DiameterEnum, Diameter>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Diameter::INCH_525);
    
    static const char *_key(Diameter value)
    {
        switch (value) {
                
            case Diameter::INCH_35:     return "INCH_35";
            case Diameter::INCH_525:    return "INCH_525";
        }
        return "???";
    }
    static const char *help(Diameter value)
    {
        switch (value) {
                
            case Diameter::INCH_35:     return "3.5\" floppy disk";
            case Diameter::INCH_525:    return "5.25\" floppy disk";
        }
        return "???";
    }
};

enum class Density
{
    SD,
    DD,
    HD
};

struct DensityEnum : Reflection<DensityEnum, Density>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Density::HD);
    
    static const char *_key(Density value)
    {
        switch (value) {
                
            case Density::SD:     return "SD";
            case Density::DD:     return "DD";
            case Density::HD:     return "HD";
        }
        return "???";
    }
    static const char *help(Density value)
    {
        switch (value) {
                
            case Density::SD:     return "Single Density";
            case Density::DD:     return "Double Density";
            case Density::HD:     return "High Density";
        }
        return "???";
    }
};


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

}
