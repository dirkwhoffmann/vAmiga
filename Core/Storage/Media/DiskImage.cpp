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
DiskImage::read(u8 *dst, isize offset, isize count)
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

/*
void
DiskImage::readSector(u8 *dst, isize s) const
{
    isize sectorSize = bsize();
    isize offset = s * sectorSize;

    assert(dst != nullptr);
    assert(offset + sectorSize <= data.size);

    for (isize i = 0; i < sectorSize; i++) {
        dst[i] = data[offset + i];
    }
}
*/

void
DiskImage::readSector(u8 *dst, isize t, isize s) const
{
    assert(0 <= t && t < numTracks());

    memcpy((void *)dst, (void *)(data.ptr + boffset(TS{t,s})), bsize());
}

/*
void
DiskImage::writeByte(isize b, isize offset, u8 value)
{
    data[b * bsize() + offset] = value;
}

void
DiskImage::writeByte(isize t, isize s, isize offset, u8 value)
{
    writeByte(bindex(TS{t,s}), offset, value);

}
*/

/*
void
DiskImage::writeSector(isize b, const Buffer<u8> &buffer)
{
    isize offset = b * bsize();
    assert(offset + bsize() <= data.size);

    for (isize i = 0; i < bsize(); i++) {
        data[offset + i] = buffer[i];
    }
}

void
DiskImage::writeSector(isize t, isize s, const Buffer<u8> &buffer)
{
    assert(0 <= t && t < numTracks());

    memcpy((void *)buffer.ptr, (void *)(data.ptr + nr * bsize()), bsize());
}
*/

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
