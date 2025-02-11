// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "HDZFile.h"

namespace vamiga {

bool
HDZFile::isCompatible(const std::filesystem::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".HDZ";
}

bool
HDZFile::isCompatible(const u8 *buf, isize len)
{
    return true;
}

bool
HDZFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

void
HDZFile::finalizeRead()
{
    debug(HDF_DEBUG, "Decompressing %ld bytes...\n", data.size);
    
    try {
        data.ungzip();
    } catch (std::runtime_error &err) {
        throw CoreException(CoreError::ZLIB_ERROR, err.what());
    }
    
    debug(HDF_DEBUG, "Restored %ld bytes.\n", data.size);
    
    // Initialize the ADF with the decompressed data (may throw)
    hdf.init(data.ptr, data.size);

    // Delete the original data
    data.dealloc();
}

}
