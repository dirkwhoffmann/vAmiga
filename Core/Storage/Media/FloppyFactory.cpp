// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FloppyFactory.h"
#include "MediaFile.h"
#include "ADFFactory.h"
#include "ADZFactory.h"
#include "IMGFactory.h"
#include "DMSFactory.h"
#include "EXEFactory.h"
#include "utl/io.h"

namespace vamiga {

std::unique_ptr<FloppyDiskImage>
FloppyFactory::make(const fs::path &path)
{
    std::unique_ptr<FloppyDiskImage> result;

    switch (MediaFile::type(path)) {

        case FileType::ADF:  result = ADFFactory::make(path); break;
        case FileType::ADZ:  result = ADZFactory::make(path); break;
        case FileType::IMG:  result = IMGFactory::make(path); break;
        case FileType::DMS:  result = DMSFactory::make(path); break;
        case FileType::EXE:  result = EXEFactory::make(path); break;

        default:
            throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
    }

    result->path = path;
    return result;
}

}
