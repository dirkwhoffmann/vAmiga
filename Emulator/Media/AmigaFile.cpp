// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AmigaFile.h"
#include "Snapshot.h"
#include "ADFFile.h"
#include "ExtendedRomFile.h"
#include "EXTFile.h"
#include "IMGFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "Folder.h"
#include "HDFFile.h"
#include "RomFile.h"
#include "Script.h"

void
AmigaFile::init(const string &path)
{
    std::ifstream stream(path, std::ifstream::binary);
    if (!stream.is_open()) throw VAError(ERROR_FILE_NOT_FOUND, path);
    init(path, stream);
}

void
AmigaFile::init(const string &path, std::istream &stream)
{
    if (!isCompatiblePath(path)) throw VAError(ERROR_FILE_TYPE_MISMATCH);
    init(stream);
    this->path = path;
}

void
AmigaFile::init(std::istream &stream)
{
    if (!isCompatibleStream(stream)) throw VAError(ERROR_FILE_TYPE_MISMATCH);
    readFromStream(stream);
}

void
AmigaFile::init(const u8 *buf, isize len)
{    
    assert(buf);
    std::stringstream stream;
    stream.write((const char *)buf, len);
    init(stream);
}

void
AmigaFile::init(const Buffer<u8> &buffer)
{
    init(buffer.ptr, buffer.size);
}

void
AmigaFile::init(FILE *file)
{
    assert(file);
    std::stringstream stream;
    int c; while ((c = fgetc(file)) != EOF) { stream.put((char)c); }
    init(stream);
}
    
AmigaFile::~AmigaFile()
{
    // if (data) delete[] data;
}



void
AmigaFile::flash(u8 *buf, isize offset, isize len) const
{
    assert(buf);
    std::memcpy(buf + offset, data.ptr, len);
}

void
AmigaFile::flash(u8 *buf, isize offset) const
{
    flash (buf, offset, data.size);
}

void
AmigaFile::flash(u8 *buf) const
{
    flash(buf, 0);
}

FileType
AmigaFile::type(const string &path)
{
    std::ifstream stream(path, std::ifstream::binary);
    
    if (stream.is_open()) {
        
        if (Snapshot::isCompatible(path) &&
            Snapshot::isCompatible(stream)) return FILETYPE_SNAPSHOT;
        
        if (ADFFile::isCompatible(path) &&
            ADFFile::isCompatible(stream)) return FILETYPE_ADF;
        
        if (HDFFile::isCompatible(path) &&
            HDFFile::isCompatible(stream)) return FILETYPE_HDF;
        
        if (EXTFile::isCompatible(path) &&
            EXTFile::isCompatible(stream)) return FILETYPE_EXT;
        
        if (IMGFile::isCompatible(path) &&
            IMGFile::isCompatible(stream)) return FILETYPE_IMG;
        
        if (DMSFile::isCompatible(path) &&
            DMSFile::isCompatible(stream)) return FILETYPE_DMS;
        
        if (EXEFile::isCompatible(path) &&
            EXEFile::isCompatible(stream)) return FILETYPE_EXE;
        
        if (RomFile::isCompatible(path) &&
            RomFile::isCompatible(stream)) return FILETYPE_ROM;
        
        if (Folder::isCompatible(path)) return FILETYPE_DIR;
    }
    
    return FILETYPE_UNKNOWN;
}

string
AmigaFile::sizeAsString()
{
    return util::byteCountAsString(data.size);
}

isize
AmigaFile::readFromStream(std::istream &stream)
{
    // Get stream size
    auto fsize = stream.tellg();
    stream.seekg(0, std::ios::end);
    fsize = stream.tellg() - fsize;
    stream.seekg(0, std::ios::beg);

    // Allocate memory
    data.init(isize(fsize));
    data.clear();
    
    // Read from stream
    stream.read((char *)data.ptr, data.size);
    finalizeRead();

    return data.size;
}

isize
AmigaFile::readFromFile(const string &path)
{        
    std::ifstream stream(path, std::ifstream::binary);

    if (!stream.is_open()) {
        throw VAError(ERROR_FILE_CANT_READ, path);
    }

    this->path = string(path);

    isize result = readFromStream(stream);
    assert(result == data.size);
    
    return result;
}

isize
AmigaFile::readFromBuffer(const u8 *buf, isize len)
{
    assert(buf);

    // Allocate memory
    data.alloc(len);

    // Copy data
    std::memcpy(data.ptr, buf, data.size);
    finalizeRead();
    
    return data.size;
}

isize
AmigaFile::readFromBuffer(const Buffer<u8> &buffer)
{
    return readFromBuffer(buffer.ptr, buffer.size);
}

isize
AmigaFile::writeToStream(std::ostream &stream, isize offset, isize len)
{
    assert(offset >= 0 && offset < data.size);
    assert(len >= 0 && offset + len <= data.size);

    stream.write((char *)data.ptr + offset, len);
    finalizeWrite();
    
    return data.size;
}

isize
AmigaFile::writeToFile(const string &path, isize offset, isize len)
{
    if (util::isDirectory(path)) {
        throw VAError(ERROR_FILE_IS_DIRECTORY);
    }
    
    std::ofstream stream(path, std::ofstream::binary);

    if (!stream.is_open()) {
        throw VAError(ERROR_FILE_CANT_WRITE, path);
    }
    
    isize result = writeToStream(stream, offset, len);
    assert(result == data.size);
    
    return result;
}

isize
AmigaFile::writeToBuffer(u8 *buf, isize offset, isize len)
{
    assert(buf);
    assert(offset >= 0 && offset < data.size);
    assert(len >= 0 && offset + len <= data.size);

    std::memcpy(buf, (char *)data.ptr + offset, len);
    finalizeWrite();

    return data.size;
}

isize
AmigaFile::writeToBuffer(Buffer<u8> &buffer, isize offset, isize len)
{
    buffer.alloc(len);
    return writeToBuffer(buffer.ptr, offset, len);
}

isize
AmigaFile::writeToStream(std::ostream &stream)
{
    return writeToStream(stream, 0, data.size);
}

isize
AmigaFile::writeToFile(const string &path)
{
    return writeToFile(path, 0, data.size);
}

isize
AmigaFile::writeToBuffer(u8 *buf)
{
    return writeToBuffer(buf, 0, data.size);
}

isize
AmigaFile::writeToBuffer(Buffer<u8> &buffer)
{
    return writeToBuffer(buffer, 0, data.size);
}
