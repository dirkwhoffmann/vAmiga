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

namespace vamiga {

void
ADZFile::init(const class ADFFile &adf) throws
{
    this->adf = adf;
    data = adf.data;
    
    debug(HDF_DEBUG, "Uncompressed ADF size: %ld bytes\n", data.size);
    
    {   util::StopWatch(HDF_DEBUG, "Compressing ADF...");
        
        try {
            data.gzip();
        } catch (std::runtime_error &err) {
            throw AppError(Fault::ZLIB_ERROR, err.what());
        }
    }
    
    debug(HDF_DEBUG, "Compressed ADF size: %ld bytes.\n", data.size);
}

bool
ADZFile::isCompatible(const fs::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".ADZ";
}

bool
ADZFile::isCompatible(const u8 *buf, isize len)
{
    return true;
}

bool
ADZFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

void
ADZFile::finalizeRead()
{
    debug(ADF_DEBUG, "Decompressing %ld bytes...\n", data.size);
    
    try {
        data.gunzip();
    } catch (std::runtime_error &err) {
        throw AppError(Fault::ZLIB_ERROR, err.what());
    }
    
    debug(ADF_DEBUG, "Restored %ld bytes.\n", data.size);
    
    // Initialize the ADF with the decompressed data (may throw)
    adf.init(data.ptr, data.size);

    // Delete the original data
    data.dealloc();
}

}
