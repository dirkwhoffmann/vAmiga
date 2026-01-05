// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IMGFile.h"
#include "FloppyDrive.h"
#include "DeviceError.h"
#include "utl/io.h"
#include "utl/support/Strings.h"
#include <format>

namespace retro::vault::image {

using retro::vault::device::DeviceError;

optional<ImageInfo>
IMGFile::about(const fs::path &path)
{
    // Check suffix
    auto suffix = utl::uppercased(path.extension().string());
    if (suffix != ".IMG") return {};

    // Check file size
    auto size = utl::getSizeOfFile(path);
    if (size != IMGSIZE_35_DD) return {};

    return {{ ImageType::FLOPPY, ImageFormat::IMG }};
}

void
IMGFile::init(Diameter dia, Density den)
{
    if (dia != Diameter::INCH_35 || den != Density::DD) {

        // We only support 3.5"DD disks at the moment
        throw DeviceError(DeviceError::DSK_INVALID_LAYOUT);
    }

    init(9 * 160 * 512);
}

std::vector<string>
IMGFile::describe() const noexcept
{
    return {
        "PC Floppy Disk",
        std::format("{} {}",
                    getDiameterStr(), getDensityStr()),
        std::format("{} Cylinders, {} Sides, {} Sectors",
                    numCyls(), numHeads(), numSectors())
    };
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

BitView
IMGFile::encode(TrackNr t) const
{
    throw std::runtime_error("NOT IMPLEMENTED YET");
}

void
IMGFile::decode(TrackNr t, BitView bits)
{
    throw std::runtime_error("NOT IMPLEMENTED YET");
}

}
