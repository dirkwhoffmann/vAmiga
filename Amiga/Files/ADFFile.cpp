// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


#include "ADFFile.h"

ADFFile::ADFFile()
{
    setDescription("ADFFile");
}

bool
ADFFile::isADFBuffer(const uint8_t *buffer, size_t length)
{
    // There are no magic bytes. We can only check the buffer size.
    return length == 90120;
}

bool
ADFFile::isADFFile(const char *path)
{
    // There are no magic bytes. We can only check the file size.
    return checkFileSize(path, 90120, 90120);
}

ADFFile *
ADFFile::makeWithBuffer(const uint8_t *buffer, size_t length)
{
    ADFFile *adf = new ADFFile();
    
    if (!adf->readFromBuffer(buffer, length)) {
        delete adf;
        return NULL;
    }
    
    return adf;
}

ADFFile *
ADFFile::makeWithFile(const char *path)
{
    ADFFile *adf = new ADFFile();
    
    if (!adf->readFromFile(path)) {
        delete adf;
        return NULL;
    }
    
    return adf;
}

bool
ADFFile::readFromBuffer(const uint8_t *buffer, size_t length)
{
    if (!VAFile::readFromBuffer(buffer, length))
        return false;
    
    return isADFBuffer(buffer, length);
}
