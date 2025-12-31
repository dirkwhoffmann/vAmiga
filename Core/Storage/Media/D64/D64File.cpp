// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "D64File.h"
#include "utl/io.h"
#include "utl/support/Strings.h"

namespace vamiga {

bool
D64File::isCompatible(const fs::path &path)
{
    // Check suffix
    if (utl::uppercased(path.extension().string()) != ".D64") return false;

    // Check file size
    auto len = utl::getSizeOfFile(path);

    return
    len == D64_683_SECTORS ||
    len == D64_683_SECTORS_ECC ||
    len == D64_768_SECTORS ||
    len == D64_768_SECTORS_ECC ||
    len == D64_802_SECTORS ||
    len == D64_802_SECTORS_ECC;
}

isize
D64File::numCyls() const
{
    switch (data.size) {

        case D64_683_SECTORS:
        case D64_683_SECTORS_ECC:   return 35;
        case D64_768_SECTORS:
        case D64_768_SECTORS_ECC:   return 40;
        case D64_802_SECTORS:
        case D64_802_SECTORS_ECC:   return 42;

        default:
            fatalError;
    }
}

isize
D64File::numHeads() const
{
    return 1;
}

isize
D64File::numSectors() const
{
    switch (data.size) {

        case D64_683_SECTORS:
        case D64_683_SECTORS_ECC:   return 683;
        case D64_768_SECTORS:
        case D64_768_SECTORS_ECC:   return 768;
        case D64_802_SECTORS:
        case D64_802_SECTORS_ECC:   return 802;

        default:
            fatalError;
    }
}

Diameter
D64File::getDiameter() const
{
    return Diameter::INCH_525;
}

Density
D64File::getDensity() const
{
    return Density::SD;
}

bool
D64File::hasEcc() const
{
    switch (data.size) {

        case D64_683_SECTORS_ECC: return true;
        case D64_768_SECTORS_ECC: return true;
        case D64_802_SECTORS_ECC: return true;

        default:
            return false;
    }
}

optional<span<const u8>>
D64File::ecc() const
{
    if (hasEcc())
        return span<const u8>(data.ptr + bsize() * numSectors(), numSectors());

    return {};
}

}
