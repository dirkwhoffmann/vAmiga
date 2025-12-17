// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DiskFile.h"
#include "utl/support/Strings.h"

namespace vamiga {

u8
DiskFile::readByte(isize b, isize offset) const
{
    assert(offset < bsize());
    return data[b * 512 + offset];
}

u8
DiskFile::readByte(isize t, isize s, isize offset) const
{
    return readByte(t * numSectors() + s, offset);
}

void
DiskFile::readSector(u8 *dst, isize s) const
{
    isize sectorSize = bsize();
    isize offset = s * sectorSize;

    assert(dst != nullptr);
    assert(offset + sectorSize <= data.size);

    for (isize i = 0; i < sectorSize; i++) {
        dst[i] = data[offset + i];
    }
}

void
DiskFile::readSector(u8 *dst, isize t, isize s) const
{
    readSector(dst, t * numSectors() + s);
}

void
DiskFile::writeByte(isize b, isize offset, u8 value)
{
    assert(offset < bsize());
    data[b * 512 + offset] = value;
}

void
DiskFile::writeByte(isize t, isize s, isize offset, u8 value)
{
    writeByte(t * numSectors() + s, offset, value);

}

void
DiskFile::writeSector(isize b, const Buffer<u8> &buffer)
{
    isize offset = b * bsize();
    assert(offset + bsize() <= data.size);

    for (isize i = 0; i < bsize(); i++) {
        data[offset + i] = buffer[i];
    }
}

void
DiskFile::writeSector(isize t, isize s, const Buffer<u8> &buffer)
{
    writeSector(t * numSectors() + s, buffer);
}

string
DiskFile::describeGeometry()
{
    return
    std::to_string(numCyls()) + " - " +
    std::to_string(numHeads()) + " - " +
    std::to_string(numSectors());
}

string
DiskFile::describeCapacity()
{
    return utl::byteCountAsString(numBytes());
}

string
DiskFile::hexdump(isize b, isize offset, isize len) const
{
    string result;
    auto p = data.ptr + b * bsize();
    
    for (isize i = 0; i < len; i++) {
        result += (i == 0 ? "" : " ") + utl::hexstr<2>(p[i]);
    }

    return result;
}

string
DiskFile::hexdump(isize t, isize s, isize offset, isize len) const
{
    return hexdump(t * numSectors() + s, offset, len);
}

string
DiskFile::hexdump(isize c, isize h, isize s, isize offset, isize len) const
{
    return hexdump(c * numHeads() + h, s, offset, len);
}

string
DiskFile::asciidump(isize b, isize offset, isize len) const
{
    string result;
    auto p = data.ptr + b * bsize() + offset;

    for (isize i = 0; i < len; i++) {
        result += isprint(int(p[i])) ? char(p[i]) : '.';
    }
    
    return result;
}

string
DiskFile::asciidump(isize t, isize s, isize offset, isize len) const
{
    return asciidump(t * numSectors() + s, offset, len);
}

string
DiskFile::asciidump(isize c, isize h, isize s, isize offset, isize len) const
{
    return asciidump(c * numHeads() + h, s, offset, len);
}

}
