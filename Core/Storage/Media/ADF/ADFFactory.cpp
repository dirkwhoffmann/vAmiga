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
#include "DiskEncoder.h"
#include "FileSystem.h"
#include "FloppyDisk.h"
#include "FloppyDrive.h"

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
    DiameterEnum::validate(dia);
    DensityEnum::validate(den);
    
    return std::make_unique<ADFFile>(ADFFile::fileSize(dia, den));
}

std::unique_ptr<ADFFile>
ADFFactory::make(const GeometryDescriptor &descr)
{
    auto bytes = descr.numBytes();

    switch (bytes) {

        case ADFFile::ADFSIZE_35_DD:
        case ADFFile::ADFSIZE_35_DD_81:
        case ADFFile::ADFSIZE_35_DD_82:
        case ADFFile::ADFSIZE_35_DD_83:
        case ADFFile::ADFSIZE_35_DD_84:
        case ADFFile::ADFSIZE_35_HD:

            return std::make_unique<ADFFile>(bytes);

        default:
            break;
    }

    throw DeviceError(DeviceError::DSK_INVALID_LAYOUT);
}

std::unique_ptr<ADFFile>
ADFFactory::make(const class FloppyDisk &disk)
{
    auto adf = make(disk.getDiameter(), disk.getDensity());

    assert(adf->numTracks() == 160);
    assert(adf->numSectors() == 11 || adf->numSectors() == 22);

    DiskEncoder::decode(*adf, disk);
    return adf;
}

std::unique_ptr<ADFFile>
ADFFactory::make(const class FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw DeviceError(DeviceError::DSK_MISSING);
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
            throw FSError(FSError::FS_WRONG_CAPACITY);
    }

    volume.exporter.exportVolume(adf->data.ptr, adf->data.size);

    return adf;
}

}
