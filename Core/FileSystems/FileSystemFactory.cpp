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
#include "ADFFile.h"
#include "HDFFile.h"

namespace vamiga {

FileSystem
FileSystemFactory::fromMediaFile(const MediaFile &file, isize part)
{
    switch (file.type()) {

        case FileType::ADF:

            return fromADF(dynamic_cast<const ADFFile &>(file));
            break;

        case FileType::HDF:

            return fromHDF(dynamic_cast<const HDFFile &>(file), part);
            break;

        default:
            throw AppError(Fault::FILE_TYPE_UNSUPPORTED);
    }
}

FileSystem
FileSystemFactory::fromADF(const ADFFile &adf) {

    auto desc = adf.getFileSystemDescriptor();
    return FileSystem(desc, adf.data.ptr, desc.numBlocks * 512);
}

FileSystem
FileSystemFactory::fromHDF(const HDFFile &hdf, isize part)
{
    auto desc = hdf.getFileSystemDescriptor(part);
    return FileSystem(desc, hdf.partitionData(part), hdf.partitionSize(part));
}

FileSystem
FileSystemFactory::fromFloppyDrive(const FloppyDrive &dfn)
{
    return fromADF(ADFFile(dfn));
}

FileSystem
FileSystemFactory::fromHardDrive(const HardDrive &hdn, isize part)
{
    return fromHDF(HDFFile(hdn), part);
}

FileSystem
FileSystemFactory::createEmpty(isize capacity, isize blockSize)
{
    return FileSystem(capacity, blockSize);
}

FileSystem
FileSystemFactory::createFromDescriptor(const FSDescriptor &desc,
                                                   const fs::path &path)
{
    return FileSystem(desc, path);
}

FileSystem
FileSystemFactory::createLowLevel(Diameter dia,
                                  Density den,
                                  FSFormat dos,
                                  const fs::path &path)
{
    return FileSystem(dia, den, dos, path);
}

}
