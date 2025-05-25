// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "FloppyFile.h"
#include "ADFFile.h"
#include "ADZFile.h"
#include "IMGFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "StringUtils.h"

namespace vamiga {

FloppyFile *
FloppyFile::make(const fs::path &path)
{
    FloppyFile *result = nullptr;

    if (!fs::exists(path)) {
        throw CoreError(Fault::FILE_NOT_FOUND, path);
    }

    Buffer<u8> buffer(path);
    
    if (buffer.empty()) {
        throw CoreError(Fault::FILE_CANT_READ, path);
    }

    switch (type(path)) {

        case FileType::ADF:  result = new ADFFile(buffer.ptr, buffer.size); break;
        case FileType::ADZ:  result = new ADZFile(buffer.ptr, buffer.size); break;
        case FileType::IMG:  result = new IMGFile(buffer.ptr, buffer.size); break;
        case FileType::DMS:  result = new DMSFile(buffer.ptr, buffer.size); break;
        case FileType::EXE:  result = new EXEFile(buffer.ptr, buffer.size); break;

        default:
            throw CoreError(Fault::FILE_TYPE_UNSUPPORTED);
    }

    result->path = path;
    return result;
}

FloppyDiskDescriptor
FloppyFile::getDescriptor() const
{
    return FloppyDiskDescriptor {

        .diameter = getDiameter(),
        .density = getDensity(),
        .sides = numHeads(),
        .cylinders = numCyls()
    };
}

}
