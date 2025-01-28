// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AmigaFile.h"
#include "Snapshot.h"
#include "ADFFile.h"
#include "ExtendedRomFile.h"
#include "EADFFile.h"
#include "IMGFile.h"
#include "STFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "Folder.h"
#include "HDFFile.h"
#include "RomFile.h"
#include "Script.h"

namespace vamiga {

void
AmigaFile::init(isize len)
{
    data.init(len);
}

void
AmigaFile::init(const Buffer<u8> &buffer)
{
    init(buffer.ptr, buffer.size);
}

void
AmigaFile::init(const string &str)
{
    init((const u8 *)str.c_str(), (isize)str.length());
}

void
AmigaFile::init(const std::filesystem::path &path)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) throw VAException(VAError::FILE_NOT_FOUND, path);

    std::ostringstream sstr(std::ios::binary);
    sstr << stream.rdbuf();
    init(sstr.str());
    this->path = path;
}

void
AmigaFile::init(const u8 *buf, isize len)
{    
    assert(buf);
    if (!isCompatibleBuffer(buf, len)) throw VAException(VAError::FILE_TYPE_MISMATCH);
    readFromBuffer(buf, len);
}

AmigaFile::~AmigaFile()
{

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

bool
AmigaFile::isCompatibleBuffer(const Buffer<u8> &buffer)
{
    return isCompatibleBuffer(buffer.ptr, buffer.size);
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
AmigaFile::writeToFile(const std::filesystem::path &path, isize offset, isize len)
{
    if (util::isDirectory(path)) {
        throw VAException(VAError::FILE_IS_DIRECTORY);
    }
    
    std::ofstream stream(path, std::ofstream::binary);

    if (!stream.is_open()) {
        throw VAException(VAError::FILE_CANT_WRITE, path);
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
AmigaFile::writeToFile(const std::filesystem::path &path)
{
    return writeToFile(path, 0, data.size);
}

isize 
AmigaFile::writePartitionToFile(const std::filesystem::path &path, isize partition)
{
    throw VAException(VAError::FILE_TYPE_UNSUPPORTED);
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

}
