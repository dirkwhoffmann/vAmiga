// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ADFFactory.h"
#include "FloppyDisk.h"
#include "FloppyDrive.h"
// #include <utility>

namespace vamiga {

std::unique_ptr<ADFFile>
ADFFactory::make(const fs::path &path)
{
    return std::make_unique<ADFFile>(path);
}

std::unique_ptr<ADFFile>
ADFFactory::make(const u8 *buf, isize len)
{
    return std::make_unique<ADFFile>(buf, len);
}

std::unique_ptr<ADFFile>
ADFFactory::make(Diameter dia, Density den)
{
    assert_enum(Diameter, dia);
    return std::make_unique<ADFFile>(ADFFile::fileSize(dia, den));
}

std::unique_ptr<ADFFile>
ADFFactory::make(const FloppyDiskDescriptor &descr)
{
    if (descr.diameter != Diameter::INCH_35) throw AppError(Fault::DISK_INVALID_DIAMETER);

    switch (descr.density) {

        case Density::DD:

            switch (descr.cylinders) {

                case 80: return std::make_unique<ADFFile>(ADFFile::ADFSIZE_35_DD);
                case 81: return std::make_unique<ADFFile>(ADFFile::ADFSIZE_35_DD_81);
                case 82: return std::make_unique<ADFFile>(ADFFile::ADFSIZE_35_DD_82);
                case 83: return std::make_unique<ADFFile>(ADFFile::ADFSIZE_35_DD_83);
                case 84: return std::make_unique<ADFFile>(ADFFile::ADFSIZE_35_DD_84);

                default:
                    throw AppError(Fault::DISK_INVALID_LAYOUT);
            }
            break;

        case Density::HD:

            return std::make_unique<ADFFile>(ADFFile::ADFSIZE_35_HD);
            break;

        default:
            throw AppError(Fault::DISK_INVALID_DENSITY);
    }
}

std::unique_ptr<ADFFile>
ADFFactory::make(const class FloppyDisk &disk)
{
    auto adf = make(disk.getDiameter(), disk.getDensity());

    assert(adf->numTracks() == 160);
    assert(adf->numSectors() == 11 || adf->numSectors() == 22);

    adf->decodeDisk(disk);

    return adf;
}

std::unique_ptr<ADFFile>
ADFFactory::make(const class FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw AppError(Fault::DISK_MISSING);
    return make(*drive.disk);
}

std::unique_ptr<ADFFile>
ADFFactory::make(const FileSystem &volume)
{
    std::unique_ptr<ADFFile> adf;

    switch (volume.blocks()) {

        case 2 * 880:
            adf = make(Diameter::INCH_35, Density::DD);
            break;

        case 4 * 880:
            adf = make(Diameter::INCH_35, Density::HD);
            break;

        default:
            throw AppError(Fault::FS_WRONG_CAPACITY);
    }

    volume.exporter.exportVolume(adf->data.ptr, adf->data.size);

    return adf;
}

}
