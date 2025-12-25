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

using CHS = DiskImage::CHS;
using TS  = DiskImage::TS;

void
DiskImage::buildTrackMap() const
{
    track2block.clear();
    track2block.reserve(numTracks());

    for (isize t = 0, offset = 0; t < numTracks(); ++t) {

        track2block.push_back(offset);
        offset += numSectors(t);
    }
}

isize
DiskImage::block2track(isize b) const
{
    assert(0 <= b && b < capacity());

    if (track2block.empty()) buildTrackMap();

    // Find the track via binary search
    auto it = std::upper_bound(track2block.begin(), track2block.end(), b);
    return isize(std::distance(track2block.begin(), it) - 1);
}

CHS
DiskImage::chs(isize b) const
{
    auto t = block2track(b);
    auto c = t / numHeads();
    auto h = t % numHeads();
    auto s = b - track2block[t];

    return { c, h, s };
}

CHS
DiskImage::chs(isize t, isize s) const
{
    assert(0 <= t && t < numTracks());
    assert(0 <= s && s < numSectors(t));

    auto c = t / numHeads();
    auto h = t % numHeads();

    return { c, h, s };
}

TS
DiskImage::ts(isize b) const
{
    assert(0 <= b && b < capacity());

    auto t = block2track(b);
    auto s = b - track2block[t];

    return { t, s };
}

TS
DiskImage::ts(isize c, isize h, isize s) const
{
    auto t = c * numHeads() + h;

    assert(0 <= t && t < numTracks());
    assert(0 <= s && s < numSectors(t));

    return { t, s };
}

isize
DiskImage::bindex(CHS chs) const
{
    return bindex(TS {chs.cylinder * numHeads() + chs.head, chs.sector});
}

isize
DiskImage::bindex(TS ts) const
{
    assert(0 <= ts.track && ts.track < numTracks());
    assert(0 <= ts.sector && ts.sector < numSectors(ts.track));

    if (track2block.empty()) buildTrackMap();

    return track2block[ts.track] + ts.sector;
}


u8
DiskImage::readByte(isize b, isize offset) const
{
    assert(0 <= b && b < capacity());
    assert(0 <= offset && offset < bsize());

    return data[b * bsize() + offset];
}

u8
DiskImage::readByte(isize t, isize s, isize offset) const
{
    return readByte(bindex(TS{t,s}), offset);
}


void
DiskImage::readBlock(u8 *dst, isize nr)
{
    assert(0 <= nr && nr < capacity());

    memcpy((void *)dst, (void *)(data.ptr + nr * bsize()), bsize());
}

void
DiskImage::writeBlock(const u8 *dst, isize nr)
{
    assert(0 <= nr && nr < capacity());
    assert(nr * bsize() + bsize() <= data.size);

    memcpy((void *)dst, (void *)(data.ptr + nr * bsize()), bsize());
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
