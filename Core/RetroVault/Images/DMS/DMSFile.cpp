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
#include <format>

using vamiga::MediaError;

extern "C" {
unsigned short extractDMS(const unsigned char *in, size_t inSize,
                          unsigned char **out, size_t *outSize, int verbose);
}

namespace retro::image {

optional<ImageInfo>
DMSFile::about(const fs::path &path)
{
    // Check suffix
    auto suffix = utl::uppercased(path.extension().string());
    if (suffix != ".DMS") return {};

    // Check magic bytes
    if (!utl::matchingFileHeader(path, "DMS!")) return {};

    return {{ ImageType::FLOPPY, ImageFormat::DMS }};
}

std::vector<string>
DMSFile::describe() const noexcept
{
    return {
        "Amiga Floppy Disk",
        std::format("{} {}",
                    getDiameterStr(), getDensityStr()),
        std::format("{} Cylinders, {} Sides, {} Sectors",
                    numCyls(), numHeads(), numSectors(0))
    };
}

void
DMSFile::didLoad()
{
    u8* adfData = nullptr;
    size_t adfSize = 0;

    int verbose = 0;
    if constexpr (debug::DMS_DEBUG) verbose = 1;
    if (extractDMS(data.ptr, (size_t)data.size, &adfData, &adfSize, verbose) == 0) {

        if constexpr (!force::DMS_CANT_CREATE) {
            adf.init(adfData, isize(adfSize));
        }
    }
    
    if (adfData) free(adfData);
    if (adf.empty()) throw MediaError(MediaError::DMS_CANT_CREATE);
}

}
