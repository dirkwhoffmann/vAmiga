// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

HDFFile::HDFFile()
{
}

bool
HDFFile::isHDFBuffer(const u8 *buffer, size_t length)
{
    // HDFs contain no magic bytes. We can only check the buffer size
    return length % 512 == 0;
}

bool
HDFFile::isHDFFile(const char *path)
{
    if (!checkFileSuffix(path, "hdf") &&
        !checkFileSuffix(path, "HDF")) {
        return false;
    }
    
    return getSizeOfFile(path) % 512 == 0;
}

HDFFile *
HDFFile::makeWithFile(const char *path)
{
    HDFFile *hdf = new HDFFile();
    
    if (!hdf->readFromFile(path)) {
        delete hdf;
        return nullptr;
    }
    
    return hdf;
}

HDFFile *
HDFFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    HDFFile *hdf = new HDFFile();
    
    if (!hdf->readFromBuffer(buffer, length)) {
        delete hdf;
        return nullptr;
    }
        
    return hdf;
}

bool
HDFFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isHDFBuffer(buffer, length);
}
