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

enum_long(DISK_DIAMETER)
{
    INCH_35,
    INCH_525
};
typedef DISK_DIAMETER DiskDiameter;

#ifdef __cplusplus
struct DiskDiameterEnum : util::Reflection<DiskDiameterEnum, DiskDiameter>
{
    static long minVal() { return 0; }
    static long maxVal() { return INCH_525; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }
    
    static const char *prefix() { return ""; }
    static const char *key(DiskDiameter value)
    {
        switch (value) {
                
            case INCH_35:     return "INCH_35";
            case INCH_525:    return "INCH_525";
        }
        return "???";
    }
};
#endif

enum_long(DISK_DENSITY)
{
    DISK_SD,
    DISK_DD,
    DISK_HD
};
typedef DISK_DENSITY DiskDensity;

#ifdef __cplusplus
struct DiskDensityEnum : util::Reflection<DiskDensityEnum, DiskDensity>
{
    static long minVal() { return 0; }
    static long maxVal() { return DISK_HD; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }
    
    static const char *prefix() { return "DISK"; }
    static const char *key(DiskDensity value)
    {
        switch (value) {
                
            case DISK_SD:     return "SD";
            case DISK_DD:     return "DD";
            case DISK_HD:     return "HD";
        }
        return "???";
    }
};
#endif

//
// Structures
//

#ifdef __cplusplus
struct DiskGeometry {
  
    // Physical layout parameters
    isize cylinders = 0;
    isize heads = 0;
    
    // Logical layout parameters
    isize sectors = 0;
    isize bsize = 0;
    
    DiskGeometry() { };
    DiskGeometry(DiskDiameter type, DiskDensity density);

    isize numTracks() const { return cylinders * heads; }
    isize numBlocks() const { return cylinders * heads * sectors; }
    isize numBytes() const { return cylinders * heads * sectors * bsize; }
    
    isize upperCyl() const { return cylinders ? cylinders - 1 : 0; }
    isize upperHead() const { return heads ? heads - 1 : 0; }
    isize upperTrack() const { return numTracks() ? numTracks() - 1 : 0; }
    
    bool operator==(const DiskGeometry &rhs) const
    {
        return
        this->cylinders == rhs.cylinders &&
        this->heads == rhs.heads &&
        this->sectors == rhs.sectors &&
        this->bsize == rhs.bsize;
    }

    bool operator!=(const DiskGeometry &rhs) const
    {
        return !(*this == rhs);
    }
        
    template <class W>
    void operator<<(W& worker)
    {
        worker
        
        << cylinders
        << heads
        << sectors
        << bsize;
    }
};
#endif
