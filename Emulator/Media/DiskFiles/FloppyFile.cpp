// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FloppyFile.h"
#include "ADFFile.h"
#include "IMGFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "Folder.h"
#include "StringUtils.h"

namespace vamiga {

FloppyFile *
FloppyFile::make(const std::filesystem::path &path)
{
    FloppyFile *result = nullptr;

    if (!std::filesystem::exists(path)) {
        throw Error(VAERROR_FILE_NOT_FOUND, path);
    }

    Buffer<u8> buffer(path);
    
    if (buffer.empty()) {
        throw Error(VAERROR_FILE_CANT_READ, path);
    }

    switch (type(path)) {

        case FILETYPE_ADF:  result = new ADFFile(buffer.ptr, buffer.size); break;
        case FILETYPE_IMG:  result = new IMGFile(buffer.ptr, buffer.size); break;
        case FILETYPE_DMS:  result = new DMSFile(buffer.ptr, buffer.size); break;
        case FILETYPE_EXE:  result = new EXEFile(buffer.ptr, buffer.size); break;
        case FILETYPE_DIR:  result = new Folder(path);

        default:
            throw Error(VAERROR_FILE_TYPE_MISMATCH);
    }

    result->path = path;
    return result;

    /*
    std::ifstream stream(path, std::ifstream::binary);
    if (!stream.is_open()) throw Error(VAERROR_FILE_NOT_FOUND, path);
    
    switch (type(path)) {
            
        case FILETYPE_ADF:  return new ADFFile(path, stream);
        case FILETYPE_IMG:  return new IMGFile(path, stream);
        case FILETYPE_DMS:  return new DMSFile(path, stream);
        case FILETYPE_EXE:  return new EXEFile(path, stream);
        case FILETYPE_DIR:  return new Folder(path);

        default:
            break;
    }

    throw Error(VAERROR_FILE_TYPE_MISMATCH);
    */
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
