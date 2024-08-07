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
ExtendedRomFile::isCompatible(std::istream &stream)
{
    if (util::streamLength(stream) != KB(512)) return false;
    
    return
    util::matchingStreamHeader(stream, magicBytes1, sizeof(magicBytes1)) ||
    util::matchingStreamHeader(stream, magicBytes2, sizeof(magicBytes2));
}

bool
ExtendedRomFile::isExtendedRomFile(const std::filesystem::path &path)
{
    std::ifstream stream(path, std::ifstream::binary);
    return stream.is_open() ? isCompatible(stream) : false;
}

}
