
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

struct Geometry {
  
    // Constants
    static constexpr isize cMin = HDR_C_MIN;
    static constexpr isize cMax = HDR_C_MAX;
    static constexpr isize hMin = HDR_H_MIN;
    static constexpr isize hMax = HDR_H_MAX;
    static constexpr isize sMin = HDR_S_MIN;
    static constexpr isize sMax = HDR_S_MAX;
    
    // Disk geometry (CHS)
    isize cylinders = 0;
    isize heads = 0;
    isize sectors = 0;

    // Size of a sector in bytes
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
    static std::vector<Geometry> driveGeometries(isize capacity);
    
    // Checks whether the geometry is unique
    bool unique() const;
    
    Geometry() { };
    Geometry(isize size);
    Geometry(isize c, isize h, isize s, isize b);
    Geometry(Diameter type, Density density);

    // Operators
    bool operator == (const Geometry &rhs) const;
    bool operator != (const Geometry &rhs) const;
    bool operator < (const Geometry &rhs) const;
        
    // Computed values
    isize numTracks() const { return cylinders * heads; }
    isize numBlocks() const { return cylinders * heads * sectors; }
    isize numBytes() const { return cylinders * heads * sectors * bsize; }
    isize upperCyl() const { return cylinders ? cylinders - 1 : 0; }
    isize upperHead() const { return heads ? heads - 1 : 0; }
    isize upperTrack() const { return numTracks() ? numTracks() - 1 : 0; }

    // Prints debug information
    void dump() const;
    void dump(std::ostream& os) const;

    // Throws an exception if the geometry contains unsupported values
    void checkCompatibility() const;
};
