// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DriveDescriptors.h"
#include "Error.h"
#include "IOUtils.h"
#include <vector>

bool
Geometry::operator == (const Geometry &rhs) const
{
    return
    this->cylinders == rhs.cylinders &&
    this->heads == rhs.heads &&
    this->sectors == rhs.sectors &&
    this->bsize == rhs.bsize;
}

bool
Geometry::operator != (const Geometry &rhs) const
{
    return !(*this == rhs);
}

bool
Geometry::operator < (const Geometry &rhs) const
{
    return cylinders < rhs.cylinders;
}

std::vector<Geometry>
Geometry::driveGeometries(isize capacity)
{
    std::vector<Geometry> result;
    
    // Typical number of sectors per track
    // https://www.win.tue.nl/~aeb/linux/hdtypes/hdtypes-4.html
    
    static i8 sizes[] = {
        
        16, 17, 24, 26, 27, 28, 29, 32, 34,
        35, 36, 38, 47, 50, 51, 52, 53, 55,
        56, 59, 60, 61, 62, 63
    };
    
    // Compute all geometries compatible with the file size
    for (isize h = Geometry::hMin; h <= Geometry::hMax; h++) {
        for (isize i = 0; i < isizeof(sizes); i++) {
                  
            auto s = isize(sizes[i]);
            auto cylSize = h * s * 512;
            
            if (capacity % cylSize == 0) {
                
                auto c = capacity / cylSize;

                if (c > Geometry::cMax) continue;
                if (c < Geometry::cMin && h > 1) continue;
                
                result.push_back(Geometry(c, h, s, 512));
            }
        }
    }

    // Sort all entries
    std::sort(result.begin(), result.end());
    
    return result;
}

bool
Geometry::unique() const
{
    return driveGeometries(numBytes()).size() == 1;
}

void
Geometry::dump() const
{
    dump(std::cout);
}

void
Geometry::dump(std::ostream& os) const
{
    using namespace util;
    
    os << tab("Geometry");
    os << dec(cylinders) << " - ";
    os << dec(heads) << " - ";
    os << dec(sectors) << std::endl;
}

void
Geometry::checkCompatibility() const
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

