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

//
// GeometryDescriptor
//

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

//
// PartitionDescriptor
//

PartitionDescriptor::PartitionDescriptor(const Geometry &geo) // : PartitionDescriptor()
{
    sizeBlock   = u32(geo.bsize / 4);
    heads       = u32(geo.heads);
    sectors     = u32(geo.sectors);
    lowCyl      = 0;
    highCyl     = u32(geo.upperCyl());
}

void
PartitionDescriptor::dump() const
{
    dump(std::cout);
}

void
PartitionDescriptor::dump(std::ostream& os) const
{
    using namespace util;
    
    os << tab("Name");
    os << name << std::endl;
    os << tab("Flags");
    os << dec(flags) << std::endl;
    os << tab("SizeBlock");
    os << dec(sizeBlock) << std::endl;
    os << tab("Heads");
    os << dec(heads) << std::endl;
    os << tab("Sectors");
    os << dec(sectors) << std::endl;
    os << tab("Reserved");
    os << dec(reserved) << std::endl;
    os << tab("Interleave");
    os << dec(interleave) << std::endl;
    os << tab("LowCyl");
    os << dec(lowCyl) << std::endl;
    os << tab("HighCyl");
    os << dec(highCyl) << std::endl;
    os << tab("NumBuffers");
    os << dec(numBuffers) << std::endl;
    os << tab("BufMemType");
    os << dec(bufMemType) << std::endl;
    os << tab("MaxTransfer");
    os << dec(maxTransfer) << std::endl;
    os << tab("Mask");
    os << dec(mask) << std::endl;
    os << tab("BootPrio");
    os << dec(bootPri) << std::endl;
    os << tab("DosType");
    os << dec(dosType) << std::endl;
}

void PartitionDescriptor::checkCompatibility() const
{
    if (4 * sizeBlock != 512 || FORCE_HDR_INVALID_BSIZE) {
        throw VAError(ERROR_HDR_INVALID_BSIZE);
    }
}


//
// HdrvDescriptor
//

HdrvDescriptor::HdrvDescriptor(const Geometry &geo)
{
    dump(std::cout);
}

void
HdrvDescriptor::dump() const
{
    
}

void
HdrvDescriptor::dump(std::ostream& os) const
{
    using namespace util;
    
    geometry.dump(os);
    
    os << tab("Disk vendor");
    os << dskVendor << std::endl;
    os << tab("Disk Product");
    os << dskProduct << std::endl;
    os << tab("Disk Revision");
    os << dskRevision << std::endl;
    os << tab("Controller vendor");
    os << conVendor << std::endl;
    os << tab("Controller Product");
    os << conProduct << std::endl;
    os << tab("Controller Revision");
    os << conRevision << std::endl;
}

void
HdrvDescriptor::checkCompatibility() const
{
    geometry.checkCompatibility();
}
