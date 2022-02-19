// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Geometry.h"
#include <vector>

bool
DiskGeometry::operator == (const DiskGeometry &rhs) const
{
    return
    this->cylinders == rhs.cylinders &&
    this->heads == rhs.heads &&
    this->sectors == rhs.sectors &&
    this->bsize == rhs.bsize;
}

bool
DiskGeometry::operator != (const DiskGeometry &rhs) const
{
    return !(*this == rhs);
}

bool
DiskGeometry::operator < (const DiskGeometry &rhs) const
{
    return cylinders < rhs.cylinders;
}

std::vector<DiskGeometry>
DiskGeometry::driveGeometries(isize capacity)
{
    std::vector<DiskGeometry> result;
    
    // Typical number of sectors per track
    // https://www.win.tue.nl/~aeb/linux/hdtypes/hdtypes-4.html
    
    static i8 sizes[] = {
        
        16, 17, 24, 26, 27, 28, 29, 32, 34,
        35, 36, 38, 47, 50, 51, 52, 53, 55,
        56, 59, 60, 61, 62, 63
    };
    
    // Compute all geometries compatible with the file size
    for (isize h = DiskGeometry::hMin; h <= DiskGeometry::hMin; h++) {
        for (isize i = 0; i < isizeof(sizes); i++) {
                  
            auto s = isize(sizes[i]);
            auto cylSize = h * s * 512;
            
            if (capacity % cylSize == 0) {
                
                auto c = capacity / cylSize;

                if (c > DiskGeometry::cMax) continue;
                if (c < DiskGeometry::cMin && h > 1) continue;
                
                result.push_back(DiskGeometry(c, h, s, 512));
            }
        }
    }

    // Sort all entries
    std::sort(result.begin(), result.end());
    
    return result;
}

void
DiskGeometry::checkCompatibility() const
{
    if (numBytes() > MB(504) || FORCE_HDR_TOO_LARGE) {
        throw VAError(ERROR_HDR_TOO_LARGE);
    }
    if (bsize != 512 || FORCE_HDR_INVALID_BSIZE) {
        throw VAError(ERROR_HDR_INVALID_BSIZE);
    }
    if (cylinders > cMax || FORCE_HDR_INVALID_GEOMETRY) {
        throw VAError(ERROR_HDR_INVALID_GEOMETRY);
    }
    if (cylinders < cMin && heads > 1) {
        throw VAError(ERROR_HDR_INVALID_GEOMETRY);
    }
    if (heads < hMin || heads > hMax) {
        throw VAError(ERROR_HDR_INVALID_GEOMETRY);
    }
    if (sectors < sMin || sectors > sMax) {
        throw VAError(ERROR_HDR_INVALID_GEOMETRY);
    }
}

