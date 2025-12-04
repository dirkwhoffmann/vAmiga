// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "STFactory.h"
#include "FloppyDisk.h"
#include "IOUtils.h"

namespace vamiga {

bool
STFile::isCompatible(const fs::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".ST";
}

bool
STFile::isCompatible(const u8 *buf, isize len)
{
    // There are no magic bytes. We can only check the buffer size
    return len == STSIZE_35_DD;
}

bool
STFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

isize
STFile::numCyls() const
{
    return 80;
}

isize
STFile::numHeads() const
{
    return 2;
}

isize
STFile::numSectors() const
{
    return 9;
}

}
