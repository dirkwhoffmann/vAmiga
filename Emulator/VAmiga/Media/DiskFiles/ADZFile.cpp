// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "ADZFile.h"
// #include "AnyFile.h"

/*
extern "C" {
unsigned short extractDMS(const unsigned char *in, size_t inSize,
                          unsigned char **out, size_t *outSize, int verbose);
}
*/

namespace vamiga {

bool
ADZFile::isCompatible(const std::filesystem::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".ADZ" || suffix == ".ADF.GZ";
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
    printf("Old size: %ld\n", data.size);
    
    try {
        data.ungzip();
    } catch (std::runtime_error &err) {
        throw CoreException(CoreError::ZLIB_ERROR, err.what());
    }
    
    printf("New size: %ld\n", data.size);
    
    adf.init(data.ptr, data.size);
    /*
    if (extractDMS(data.ptr, (size_t)data.size, &adfData, &adfSize, DMS_DEBUG) == 0) {

        if (!FORCE_DMS_CANT_CREATE) {
            adf.init(adfData, isize(adfSize));
        }
    }
    
    if (adfData) free(adfData);
    if (!adf) throw CoreException(CoreError::DMS_CANT_CREATE);
    */
}

}
