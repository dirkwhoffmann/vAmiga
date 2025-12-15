// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystemFactory.h"
#include "ADFFactory.h"
#include "HDFFactory.h"
#include "HDFFile.h"
#include "utl/io.h"

namespace vamiga {

std::unique_ptr<FileSystem>
FileSystemFactory::fromADF(Device &dev, const ADFFile &adf) {

    auto desc = adf.getFileSystemDescriptor();
    return make_unique<FileSystem>(dev, desc, adf.data.ptr, desc.numBlocks * 512);
}

std::unique_ptr<FileSystem>
FileSystemFactory::fromHDF(Device &dev, const HDFFile &hdf, isize part)
{
    auto desc = hdf.getFileSystemDescriptor(part);
    return make_unique<FileSystem>(dev, desc, hdf.partitionData(part), hdf.partitionSize(part));
}

std::unique_ptr<FileSystem>
FileSystemFactory::fromMediaFile(Device &dev, const MediaFile &file, isize part)
{
    switch (file.type()) {

        case FileType::ADF:

            return fromADF(dev, dynamic_cast<const ADFFile &>(*file.file));
            break;

        case FileType::HDF:

            return fromHDF(dev, dynamic_cast<const HDFFile &>(*file.file), part);
            break;

        default:
            throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
    }
}

std::unique_ptr<FileSystem>
FileSystemFactory::fromFloppyDrive(Device &dev, const FloppyDrive &dfn)
{
    return fromADF(dev, *ADFFactory::make(dfn));
}

std::unique_ptr<FileSystem>
FileSystemFactory::fromHardDrive(Device &dev, const HardDrive &hdn, isize part)
{
    return fromHDF(dev, *HDFFactory::make(hdn), part);
}

std::unique_ptr<FileSystem>
FileSystemFactory::createEmpty(Device &dev, isize capacity, isize blockSize)
{
    return make_unique<FileSystem>(dev, capacity, blockSize);
}

std::unique_ptr<FileSystem>
FileSystemFactory::createFromDescriptor(Device &dev, const FSDescriptor &desc,
                                                   const fs::path &path)
{
    return make_unique<FileSystem>(dev, desc, path);
}

std::unique_ptr<FileSystem>
FileSystemFactory::createLowLevel(Device &dev,
                                  Diameter dia,
                                  Density den,
                                  FSFormat dos,
                                  const fs::path &path)
{
    return make_unique<FileSystem>(dev, FSDescriptor(dia, den, dos), path);
}

void
FileSystemFactory::initFromADF(FileSystem &fs, const ADFFile &adf)
{
    auto desc = adf.getFileSystemDescriptor();
    fs.init(desc, adf.data.ptr, desc.numBlocks * 512);
}

void
FileSystemFactory::initFromHDF(FileSystem &fs, const HDFFile &hdf, isize part)
{
    auto desc = hdf.getFileSystemDescriptor(part);
    fs.init(desc, hdf.partitionData(part), hdf.partitionSize(part));
}

void
FileSystemFactory::initFromMedia(FileSystem &fs, const MediaFile &file, isize part)
{
    switch (file.type()) {

        case FileType::ADF:

            initFromADF(fs, dynamic_cast<const ADFFile &>(file));
            break;

        case FileType::HDF:

            initFromHDF(fs, dynamic_cast<const HDFFile &>(file), part);
            break;

        default:
            throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
    }
}

void
FileSystemFactory::initFromFloppy(FileSystem &fs, const FloppyDrive &dfn)
{
    initFromADF(fs, *ADFFactory::make(dfn));
}

void
FileSystemFactory::initFromHardDrive(FileSystem &fs, const HardDrive &hdn, isize part)
{
    initFromHDF(fs, *HDFFactory::make(hdn));
}

void
FileSystemFactory::initCreateEmpty(FileSystem &fs, isize capacity, isize blockSize)
{
    fs.init(capacity, blockSize);
}

void
FileSystemFactory::initFromDescriptor(FileSystem &fs, const FSDescriptor &desc, const fs::path &path)
{
    fs.init(desc, path);
}
void
FileSystemFactory::initLowLevel(FileSystem &fs, Diameter dia, Density den, FSFormat dos, const fs::path &path)
{
    fs.init(FSDescriptor(dia, den, dos), path);

}

}
