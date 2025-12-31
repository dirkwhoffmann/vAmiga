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

#include "ADFFile.h"
#include "ADZFile.h"
#include "EADFFile.h"
#include "HDFFile.h"
#include "HDZFile.h"
#include "IMGFile.h"
#include "STFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "D64File.h"

namespace vamiga {

using CHS = TrackDevice::CHS;
using TS  = TrackDevice::TS;

optional<ImageInfo>
DiskImage::scan(fs::path url)
{
    if (auto info = ADFFile::isCompatible(url))  return info;
    if (auto info = ADZFile::isCompatible(url))  return info;
    if (auto info = EADFFile::isCompatible(url)) return info;
    if (auto info = HDFFile::isCompatible(url))  return info;
    if (auto info = HDZFile::isCompatible(url))  return info;
    if (auto info = IMGFile::isCompatible(url))  return info;
    if (auto info = STFile::isCompatible(url))   return info;
    if (auto info = DMSFile::isCompatible(url))  return info;
    if (auto info = EXEFile::isCompatible(url))  return info;
    if (auto info = D64File::isCompatible(url))  return info;

    return {};
}

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

ByteView
DiskImage::byteView(TrackNr t) const
{
    return ByteView(data.ptr + boffset(TS{t,0}), numSectors(t) * bsize());
}

ByteView
DiskImage::byteView(TrackNr t, SectorNr s) const
{
    return ByteView(data.ptr + boffset(TS{t,s}), bsize());
}

MutableByteView
DiskImage::byteView(TrackNr t)
{
    return MutableByteView(data.ptr + boffset(TS{t,0}), numSectors(t) * bsize());
}

MutableByteView
DiskImage::byteView(TrackNr t, SectorNr s)
{
    return MutableByteView(data.ptr + boffset(TS{t,s}), bsize());
}

}
