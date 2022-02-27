
// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Constants.h"
#include "DiskTypes.h"

struct DiskGeometry {
  
    // Constants
    static constexpr isize cMin = HDR_C_MIN;
    static constexpr isize cMax = HDR_C_MAX;
    static constexpr isize hMin = HDR_H_MIN;
    static constexpr isize hMax = HDR_H_MAX;
    static constexpr isize sMin = HDR_S_MIN;
    static constexpr isize sMax = HDR_S_MAX;
    
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
    DiskGeometry(isize size);
    DiskGeometry(isize c, isize h, isize s, isize b);
    DiskGeometry(Diameter type, Density density);

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
