// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DiskImage.h"
#include "utl/support/Strings.h"

namespace vamiga {

using CHS = TrackDevice::CHS;
using TS  = TrackDevice::TS;

void
DiskImage::read(u8 *dst, isize offset, isize count) const
{
    assert(offset + count <= data.size);
    memcpy((void *)dst, (void *)(data.ptr + offset), count);
}

void
DiskImage::write(const u8 *src, isize offset, isize count)
{
    assert(offset + count <= data.size);
    memcpy((void *)(data.ptr + offset), (void *)src, count);
}

string
DiskImage::describeGeometry()
{
    return
    std::to_string(numCyls()) + " - " +
    std::to_string(numHeads()) + " - " +
    std::to_string(numSectors(0));
}

string
DiskImage::describeCapacity()
{
    return utl::byteCountAsString(numBytes());
}

string
DiskImage::hexdump(isize b, isize offset, isize len) const
{
    string result;
    auto p = data.ptr + b * bsize();
    
    for (isize i = 0; i < len; i++) {
        result += (i == 0 ? "" : " ") + utl::hexstr<2>(p[i]);
    }

    return result;
}

string
DiskImage::hexdump(isize t, isize s, isize offset, isize len) const
{
    return hexdump(t * numSectors(0) + s, offset, len);
}

string
DiskImage::hexdump(isize c, isize h, isize s, isize offset, isize len) const
{
    return hexdump(c * numHeads() + h, s, offset, len);
}

string
DiskImage::asciidump(isize b, isize offset, isize len) const
{
    string result;
    auto p = data.ptr + b * bsize() + offset;

    for (isize i = 0; i < len; i++) {
        result += isprint(int(p[i])) ? char(p[i]) : '.';
    }
    
    return result;
}

string
DiskImage::asciidump(isize t, isize s, isize offset, isize len) const
{
    return asciidump(t * numSectors(0) + s, offset, len);
}

string
DiskImage::asciidump(isize c, isize h, isize s, isize offset, isize len) const
{
    return asciidump(c * numHeads() + h, s, offset, len);
}

}
