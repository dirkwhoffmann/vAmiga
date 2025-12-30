// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DMSFile.h"
#include "MediaError.h"
#include "utl/io.h"
#include "utl/support/Strings.h"

extern "C" {
unsigned short extractDMS(const unsigned char *in, size_t inSize,
                          unsigned char **out, size_t *outSize, int verbose);
}

namespace vamiga {

bool
DMSFile::isCompatible(const fs::path &path)
{
    // Check suffix
    auto suffix = utl::uppercased(path.extension().string());
    if (suffix != ".DMS") return false;

    // Check magic bytes
    return utl::matchingFileHeader(path, "DMS!");
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
    if (adf.empty()) throw MediaError(MediaError::DMS_CANT_CREATE);
}

}
