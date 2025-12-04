// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IMGFactory.h"
#include "FloppyDrive.h"
#include "IOUtils.h"

namespace vamiga {

bool
IMGFile::isCompatible(const fs::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".IMG";
}

bool
IMGFile::isCompatible(const u8 *buf, isize len)
{
    // There are no magic bytes. We can only check the buffer size
    return len == IMGSIZE_35_DD;
}

bool
IMGFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

isize
IMGFile::numCyls() const
{
    return 80;
}

isize
IMGFile::numHeads() const
{
    return 2;
}

isize
IMGFile::numSectors() const
{
    return 9;
}

}
