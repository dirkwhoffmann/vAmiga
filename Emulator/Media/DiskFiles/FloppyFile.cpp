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

u8
FloppyFile::readByte(isize t, isize s, isize offset) const
{
    return readByte(t * numSectors() + s, offset);
}

u8
FloppyFile::readByte(isize b, isize offset) const
{
    assert(offset < 512);
    return data[b * 512 + offset];
}

void
FloppyFile::readSector(u8 *dst, isize t, isize s) const
{
    readSector(dst, t * numSectors() + s);
}

void
FloppyFile::readSector(u8 *dst, isize s) const
{
    isize sectorSize = 512;
    isize offset = s * sectorSize;

    assert(dst != nullptr);
    assert(offset + sectorSize <= size);

    for (isize i = 0; i < sectorSize; i++) {
        dst[i] = data[offset + i];
    }
}

void
FloppyFile::readSectorHex(char *dst, isize t, isize s, isize count) const
{
    readSectorHex(dst, t * numSectors() + s, count);
}

void
FloppyFile::readSectorHex(char *dst, isize s, isize count) const
{
    isize sectorSize = 512;
    isize offset = s * sectorSize;

    assert(dst != nullptr);

    for (isize i = 0; i < count; i++) {
        snprintf(dst + 3*i, 4, "%02X ", data[offset + i]);
    }
}
