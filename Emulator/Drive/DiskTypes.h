// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DISK_TYPES_H
#define _DISK_TYPES_H

#include "Aliases.h"

//
// Enumerations
//

enum_long(DISK_DIAMETER)
{
    INCH_35,
    INCH_525
};
typedef DISK_DIAMETER DiskDiameter;

inline bool isDiskDiameter(DiskDiameter value)
{
    return (unsigned long)value <= INCH_525;
}

inline const char *DiskDiameterName(DiskDiameter value)
{
    switch (value) {
            
        case INCH_35:   return "INCH_35";
        case INCH_525:  return "INCH_525";
    }
    return "???";
}

enum_long(DISK_DENSITY)
{
    DISK_SD,
    DISK_DD,
    DISK_HD
};
typedef DISK_DENSITY DiskDensity;

inline bool isDiskDensity(DiskDensity value)
{
    return (unsigned long)value <= DISK_HD;
}

inline const char *DiskDensityName(DiskDensity value)
{
    switch (value) {
            
        case DISK_SD:  return "SD";
        case DISK_DD:  return "DD";
        case DISK_HD:  return "HD";
    }
    return "???";
}

#endif
