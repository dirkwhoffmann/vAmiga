// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HDZFile.h"
#include "utl/chrono.h"
#include "utl/io.h"
#include "utl/support.h"

using vamiga::HDF_DEBUG;

namespace retro::image {

optional<ImageInfo>
HDZFile::about(const fs::path &path)
{
    auto suffix = utl::uppercased(path.extension().string());
    if (suffix != ".HDZ") return {};

    return {{ ImageType::HARDDISK, ImageFormat::HDZ }};
}

void
HDZFile::init(const class HDFFile &hdf)
{
    this->hdf = hdf;
    data = hdf.data;
    
    debug(HDF_DEBUG, "Uncompressed HDF size: %ld bytes\n", data.size);

    {   utl::StopWatch(HDF_DEBUG, "Compressing HDF...");
        
        try {
            data.gzip();
        } catch (std::runtime_error &err) {
            throw IOError(IOError::ZLIB_ERROR, err.what());
        }
    }
    
    debug(HDF_DEBUG, "Compressed HDF size: %ld bytes.\n", data.size);
}

std::vector<string>
HDZFile::describe() const noexcept
{
    return hdf.describe();
}

void
HDZFile::didLoad()
{
    debug(HDF_DEBUG, "Compressed size: %ld bytes.\n", data.size);
        
    {   utl::StopWatch(HDF_DEBUG, "Uncompressing...");

        try {
            data.gunzip();
        } catch (std::runtime_error &err) {
            throw IOError(IOError::ZLIB_ERROR, err.what());
        }
    }
    
    debug(HDF_DEBUG, "Uncompressed size: %ld bytes\n", data.size);
    
    // Initialize the ADF with the decompressed data (may throw)
    hdf.init(data.ptr, data.size);

    // Delete the original data
    data.dealloc();
}

isize
HDZFile::writePartitionToFile(const fs::path &path, isize nr) const
{
    auto offset = hdf.partitionOffset(nr);
    auto size = hdf.partitionSize(nr);

    // Write the partition into a buffer
    Buffer<u8> partition(hdf.data.ptr + offset, size);
    // hdf.writeToBuffer(partition, offset, size);

    // Compress the partition
    partition.gzip();
    
    // Write the compressed partition to disk
    HDFFile hdf(partition.ptr, partition.size);
    return hdf.writeToFile(path);
}

}
