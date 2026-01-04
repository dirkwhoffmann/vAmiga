// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystems/AmigaFS/FSBootBlockImage.h"
#include "EADFFile.h"
#include "FileSystems/AmigaFS/FileSystem.h"
#include "utl/io.h"
#include "utl/support/Strings.h"

namespace retro::image {

using retro::device::Diameter;
using retro::device::Density;
using retro::device::DeviceError;
using retro::device::Volume;
using retro::rfs::amiga::BlockNr;
using retro::rfs::amiga::FSName;
using retro::rfs::amiga::FSFormatEnum;
using retro::rfs::amiga::FileSystem;
using retro::rfs::amiga::FSBlock;
using retro::rfs::amiga::FSDescriptor;
using retro::rfs::amiga::FSError;

optional<ImageInfo>
ADFFile::about(const fs::path &path)
{
    // Check suffix
    auto suffix = utl::uppercased(path.extension().string());
    if (suffix != ".ADF") return {};

    // Get file size
    auto len = utl::getSizeOfFile(path);

    // Some ADFs contain an additional byte at the end. Ignore it.
    len &= ~1;

    // The size must be a multiple of the cylinder size
    if (len % 11264) return {};

    // Check some more limits
    if (len > ADFSIZE_35_DD_84 && len != ADFSIZE_35_HD) return {};

    // Make sure it's not an extended ADF
    if (EADFFile::about(path)) return {};

    return {{ ImageType::FLOPPY, ImageFormat::ADF }};
}

isize
ADFFile::fileSize(Diameter diameter, Density density)
{
    return fileSize(diameter, density, 80);
}

isize
ADFFile::fileSize(Diameter diameter, Density density, isize tracks)
{
    retro::device::DiameterEnum::validate(diameter);
    retro::device::DensityEnum::validate(density);

    if (diameter != Diameter::INCH_35) throw DeviceError(DeviceError::DSK_INVALID_DIAMETER);

    switch (density) {

        case Density::DD:

            switch (tracks) {

                case 80: return ADFSIZE_35_DD;
                case 81: return ADFSIZE_35_DD_81;
                case 82: return ADFSIZE_35_DD_82;
                case 83: return ADFSIZE_35_DD_83;
                case 84: return ADFSIZE_35_DD_84;

                default:
                    throw (DeviceError(DeviceError::DSK_INVALID_LAYOUT));
            }

        case Density::HD:

            return ADFSIZE_35_HD;

        default:
            throw DeviceError(DeviceError::DSK_INVALID_DENSITY);
    }
}

void
ADFFile::init(Diameter dia, Density den)
{
    retro::device::DiameterEnum::validate(dia);
    retro::device::DensityEnum::validate(den);

    init(ADFFile::fileSize(dia, den));
}

void
ADFFile::init(const GeometryDescriptor &descr)
{
    auto bytes = descr.numBytes();

    switch (bytes) {

        case ADFFile::ADFSIZE_35_DD:
        case ADFFile::ADFSIZE_35_DD_81:
        case ADFFile::ADFSIZE_35_DD_82:
        case ADFFile::ADFSIZE_35_DD_83:
        case ADFFile::ADFSIZE_35_DD_84:
        case ADFFile::ADFSIZE_35_HD:

            init(bytes);

        default:
            break;
    }

    throw DeviceError(DeviceError::DSK_INVALID_LAYOUT);
}

void
ADFFile::init(const FileSystem &volume)
{
    switch (volume.blocks()) {

        case 2 * 880: init(Diameter::INCH_35, Density::DD); break;
        case 4 * 880: init(Diameter::INCH_35, Density::HD); break;

        default:
            throw FSError(FSError::FS_WRONG_CAPACITY);
    }

    volume.exporter.exportVolume(data.ptr, data.size);
}

std::vector<string>
ADFFile::describe() const noexcept
{
    return {
        "Amiga Floppy Disk",
        std::format("{} {}",
                    getDiameterStr(), getDensityStr()),
        std::format("{} Cylinders, {} Sides, {} Sectors",
                    numCyls(), numHeads(), numSectors())
    };
}

void
ADFFile::didLoad()
{
    // Add some empty cylinders if the file contains less than 80
    if (data.size < ADFSIZE_35_DD) data.resize(ADFSIZE_35_DD, 0);
}

isize
ADFFile::numCyls() const noexcept
{
    switch(data.size & ~1) {
            
        case ADFSIZE_35_DD:    return 80;
        case ADFSIZE_35_DD_81: return 81;
        case ADFSIZE_35_DD_82: return 82;
        case ADFSIZE_35_DD_83: return 83;
        case ADFSIZE_35_DD_84: return 84;
        case ADFSIZE_35_HD:    return 80;
            
        default:
            fatalError;
    }
}

isize
ADFFile::numHeads() const noexcept
{
    return 2;
}

isize
ADFFile::numSectors() const noexcept
{
    switch (getDensity()) {
            
        case Density::DD: return 11;
        case Density::HD: return 22;
            
        default:
            fatalError;
    }
}

Diameter
ADFFile::getDiameter() const noexcept
{
    return Diameter::INCH_35;
}

Density
ADFFile::getDensity() const noexcept
{
    return (data.size & ~1) == ADFSIZE_35_HD ? Density::HD : Density::DD;
}

FSDescriptor
ADFFile::getFileSystemDescriptor() const noexcept
{
    FSDescriptor result;
    
    // Determine the root block location
    BlockNr root = data.size < ADFSIZE_35_HD ? 880 : 1760;

    // Determine the bitmap block location
    BlockNr bitmap = FSBlock::read32(data.ptr + root * 512 + 316);

    // Assign a default location if the bitmap block reference is invalid
    if (bitmap == 0 || bitmap >= (BlockNr)numBlocks()) bitmap = root + 1;

    // Setup the descriptor
    result.numBlocks = numBlocks();
    result.bsize = 512;
    result.numReserved = 2;
    result.rootBlock = root;
    result.bmBlocks.push_back(bitmap);
    
    return result;
}

void
ADFFile::formatDisk(FSFormat dos, BootBlockId id, string name)
{
    retro::rfs::amiga::FSFormatEnum::validate(dos);

    loginfo(ADF_DEBUG,
            "Formatting disk (%ld, %s)\n", numBlocks(), retro::rfs::amiga::FSFormatEnum::key(dos));

    // Only proceed if a file system is given
    if (dos == FSFormat::NODOS) return;

    // Create a file system
    auto vol = Volume(*this);
    auto fs = FileSystem(vol);

    // Format the file system
    fs.format(dos);
    fs.setName(FSName(name));
    fs.makeBootable(id);

    // Update the underlying ADF
    fs.flush();
}

}
