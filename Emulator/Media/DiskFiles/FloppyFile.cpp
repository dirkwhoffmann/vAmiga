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
FloppyFile::make(const string &path)
{
    std::ifstream stream(path, std::ifstream::binary);
    if (!stream.is_open()) throw VAError(ERROR_FILE_NOT_FOUND, path);
    
    switch (type(path)) {
            
        case FILETYPE_ADF:  return new ADFFile(path, stream);
        case FILETYPE_IMG:  return new IMGFile(path, stream);
        case FILETYPE_DMS:  return new DMSFile(path, stream);
        case FILETYPE_EXE:  return new EXEFile(path, stream);
        case FILETYPE_DIR:  return new Folder(path);

        default:
            break;
    }
    throw VAError(ERROR_FILE_TYPE_MISMATCH);
}

}
