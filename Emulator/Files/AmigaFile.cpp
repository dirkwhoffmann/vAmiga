// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "AmigaFile.h"
#include "Snapshot.h"
#include "ADFFile.h"
#include "EXTFile.h"
#include "IMGFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "Folder.h"
#include "HDFFile.h"
#include "RomFile.h"
#include "ExtendedRomFile.h"


AmigaFile::AmigaFile(usize capacity)
{
    data = new u8[capacity]();
    size = capacity;
}

AmigaFile::~AmigaFile()
{
    if (data) delete[] data;
}

void
AmigaFile::flash(u8 *buffer, size_t offset)
{
    assert(buffer != nullptr);
    memcpy(buffer + offset, data, size);
}

usize
AmigaFile::readFromStream(std::istream &stream)
{
    // Get stream size
    auto fsize = stream.tellg();
    stream.seekg(0, std::ios::end);
    fsize = stream.tellg() - fsize;
    stream.seekg(0, std::ios::beg);

    // Allocate memory
    assert(data == nullptr);
    data = new u8[fsize]();
    size = fsize;

    // Fix known inconsistencies
    stream.read((char *)data, size);
        
    return size;
}

usize
AmigaFile::readFromFile(const char *path)
{
    assert(path);
        
    std::ifstream stream(path);

    if (!stream.is_open()) {
        throw VAError(ERROR_FILE_CANT_READ);
    }
    
    usize result = readFromStream(stream);
    assert(result == size);
    
    this->path = string(path);
    return size;
}

usize
AmigaFile::readFromBuffer(const u8 *buf, usize len)
{
    assert(buf);

    std::istringstream stream(std::string((const char *)buf, len));
    
    usize result = readFromStream(stream);
    assert(result == size);
    return size;
}

usize
AmigaFile::writeToStream(std::ostream &stream)
{
    stream.write((char *)data, size);
    return size;
}

usize
AmigaFile::writeToStream(std::ostream &stream, ErrorCode *err)
{
    *err = ERROR_OK;
    try { return writeToStream(stream); }
    catch (VAError &exception) { *err = exception.errorCode; }
    return 0;
}

usize
AmigaFile::writeToFile(const char *path)
{
    assert(path);
        
    std::ofstream stream(path);

    if (!stream.is_open()) {
        throw VAError(ERROR_FILE_CANT_WRITE);
    }
    
    usize result = writeToStream(stream);
    assert(result == size);
    
    return size;
}

usize
AmigaFile::writeToFile(const char *path, ErrorCode *err)
{
    *err = ERROR_OK;
    try { return writeToFile(path); }
    catch (VAError &exception) { *err = exception.errorCode; }
    return 0;
}

usize
AmigaFile::writeToBuffer(u8 *buf)
{
    assert(buf);

    std::ostringstream stream;
    usize len = writeToStream(stream);
    stream.write((char *)buf, len);
    
    return len;
}

usize
AmigaFile::writeToBuffer(u8 *buf, ErrorCode *err)
{
    *err = ERROR_OK;
    try { return writeToBuffer(buf); }
    catch (VAError &exception) { *err = exception.errorCode; }
    return 0;
}
