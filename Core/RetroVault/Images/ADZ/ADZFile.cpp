// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ADZFile.h"
#include "utl/chrono.h"
#include "utl/io.h"
#include "utl/support.h"

namespace vamiga {

void
ADZFile::init(const class ADFFile &adf)
{
    this->adf = ADFFile(adf.data.ptr, adf.data.size);
    data = adf.data;

    debug(HDF_DEBUG, "Uncompressed ADF size: %ld bytes\n", data.size);
    
    {   utl::StopWatch(HDF_DEBUG, "Compressing ADF...");
        
        try {
            data.gzip();
        } catch (std::runtime_error &err) {
            throw IOError(IOError::ZLIB_ERROR, err.what());
        }
    }
    
    debug(HDF_DEBUG, "Compressed ADF size: %ld bytes.\n", data.size);
}

optional<ImageInfo>
ADZFile::isCompatible(const fs::path &path)
{
    auto suffix = utl::uppercased(path.extension().string());
    if (suffix != ".ADZ") return {};

    return {{ ImageType::FLOPPY, ImageFormat::ADZ }};
}

void
ADZFile::didLoad()
{
    debug(ADF_DEBUG, "Decompressing %ld bytes...\n", data.size);
    
    try {
        data.gunzip();
    } catch (std::runtime_error &err) {
        throw IOError(IOError::ZLIB_ERROR, err.what());
    }
    
    debug(ADF_DEBUG, "Restored %ld bytes.\n", data.size);
    
    // Initialize the ADF with the decompressed data (may throw)
    adf.init(data.ptr, data.size);

    // Delete the original data
    data.dealloc();
}

}
