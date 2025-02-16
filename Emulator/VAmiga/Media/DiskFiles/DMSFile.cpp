// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "DMSFile.h"

extern "C" {
unsigned short extractDMS(const unsigned char *in, size_t inSize,
                          unsigned char **out, size_t *outSize, int verbose);
}

namespace vamiga {

bool
DMSFile::isCompatible(const std::filesystem::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".DMS";
}

bool
DMSFile::isCompatible(const u8 *buf, isize len)
{
    return util::matchingBufferHeader(buf, "DMS!");
}

bool
DMSFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

void
DMSFile::finalizeRead()
{
    u8* adfData = nullptr;
    size_t adfSize = 0;
    
    if (extractDMS(data.ptr, (size_t)data.size, &adfData, &adfSize, DMS_DEBUG) == 0) {

        if (!FORCE_DMS_CANT_CREATE) {
            adf.init(adfData, isize(adfSize));
        }
    }
    
    if (adfData) free(adfData);
    if (!adf) throw CoreError(Fault::DMS_CANT_CREATE);
}

}
