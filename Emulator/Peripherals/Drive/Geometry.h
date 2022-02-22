
// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaObject.h"
#include "DiskTypes.h"

struct DiskGeometry {
  
    // Constants
    static constexpr isize cMin = 256;
    static constexpr isize cMax = 1024;
    static constexpr isize hMin = 1;
    static constexpr isize hMax = 16;
    static constexpr isize sMin = 16;
    static constexpr isize sMax = 63;

    // Physical layout parameters
    isize cylinders = 0;
    isize heads = 0;
    
    // Logical layout parameters
    isize sectors = 0;
    isize bsize = 512;
    
    template <class W>
    void operator<<(W& worker)
    {
        worker
        
        << cylinders
        << heads
        << sectors
        << bsize;
    }
    
    // Returns a vector with compatible geometries for a given byte count
    static std::vector<DiskGeometry> driveGeometries(isize capacity);

    // Checks whether the geometry is unique
    bool unique() const;
    
    DiskGeometry() { };
    DiskGeometry(isize c, isize h, isize s, isize b);
    DiskGeometry(DiskDiameter type, DiskDensity density);

    bool operator == (const DiskGeometry &rhs) const;
    bool operator != (const DiskGeometry &rhs) const;
    bool operator < (const DiskGeometry &rhs) const;
        
    isize numTracks() const { return cylinders * heads; }
    isize numBlocks() const { return cylinders * heads * sectors; }
    isize numBytes() const { return cylinders * heads * sectors * bsize; }
    
    isize upperCyl() const { return cylinders ? cylinders - 1 : 0; }
    isize upperHead() const { return heads ? heads - 1 : 0; }
    isize upperTrack() const { return numTracks() ? numTracks() - 1 : 0; }

    // Throws an exception if the geometry contains unsupported values
    void checkCompatibility() const;
    
};
