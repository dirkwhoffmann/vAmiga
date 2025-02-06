// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ExtendedRomFile.h"
#include "IOUtils.h"
#include "Macros.h"

namespace vamiga {

// AROS Extended ROM
const u8 ExtendedRomFile::magicBytes1[] = { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x02 };
const u8 ExtendedRomFile::magicBytes2[] = { 0x4E, 0x71, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x02 };

bool
ExtendedRomFile::isCompatible(const std::filesystem::path &name)
{
    return true;
}

bool
ExtendedRomFile::isCompatible(const u8 *buf, isize len)
{
    if (len != KB(512)) return false;

    return
    util::matchingBufferHeader(buf, magicBytes1, sizeof(magicBytes1)) ||
    util::matchingBufferHeader(buf, magicBytes2, sizeof(magicBytes2));
}

bool
ExtendedRomFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

}
