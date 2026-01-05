// -----------------------------------------------------------------------------
// This file is part of RetroVault
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
#include <format>

namespace retro::vault::image {

optional<ImageInfo>
D64File::about(const fs::path &path)
{
    // Check suffix
    if (utl::uppercased(path.extension().string()) != ".D64") return {};

    // Check file size
    auto len = utl::getSizeOfFile(path);

    bool match =
    len == D64_683_SECTORS ||
    len == D64_683_SECTORS_ECC ||
    len == D64_768_SECTORS ||
    len == D64_768_SECTORS_ECC ||
    len == D64_802_SECTORS ||
    len == D64_802_SECTORS_ECC;
    if (!match) return {};

    return {{ ImageType::FLOPPY, ImageFormat::D64 }};
}

std::vector<string>
D64File::describe() const noexcept
{
    return {
        "Commodore 64 Floppy Disk",
        std::format("{} {}",
                    getDiameterStr(), getDensityStr()),
        std::format("{} Tracks, {} Blocks",
                    numTracks(), numBlocks())
    };
}

isize
D64File::numCyls() const noexcept
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
D64File::numHeads() const noexcept
{
    return 1;
}

isize
D64File::numSectors(isize t) const noexcept
{
    if (t < 0 || t >= numTracks()) return 0;

    if (t < 17) return 21;  // Speedzone 3 (outer tracks)
    if (t < 24) return 19;  // Speedzone 2
    if (t < 30) return 18;  // Speedzone 1

    return 17;              // Speedzome 0 (inner tracks)
}

Diameter
D64File::getDiameter() const noexcept
{
    return Diameter::INCH_525;
}

Density
D64File::getDensity() const noexcept
{
    return Density::SD;
}

BitView
D64File::encode(TrackNr t) const
{
    throw std::runtime_error("NOT IMPLEMENTED YET");
}

void
D64File::decode(TrackNr t, BitView bits)
{
    throw std::runtime_error("NOT IMPLEMENTED YET");
}

bool
D64File::hasEcc() const noexcept
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
D64File::ecc() const noexcept
{
    if (hasEcc())
        return span<const u8>(data.ptr + bsize() * numBlocks(), numBlocks());

    return {};
}

}
