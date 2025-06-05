// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AnyFile.h"
#include "Snapshot.h"
#include "ADFFile.h"
#include "EADFFile.h"
#include "IMGFile.h"
#include "STFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "HDFFile.h"
#include "RomFile.h"
#include "Script.h"

namespace vamiga {

void
AnyFile::init(isize len)
{
    data.init(len);
}

void
AnyFile::init(const Buffer<u8> &buffer)
{
    init(buffer.ptr, buffer.size);
}

void
AnyFile::init(const string &str)
{
    init((const u8 *)str.c_str(), (isize)str.length());
}

void
AnyFile::init(const fs::path &path)
{
    std::ifstream stream(path, std::ios::binary);
    
    if (!stream.is_open()) {
        throw AppError(Fault::FILE_NOT_FOUND, path);
    }
    if (!isCompatiblePath(path)) {
        throw AppError(Fault::FILE_TYPE_MISMATCH, path);
    }
    std::ostringstream sstr(std::ios::binary);
    sstr << stream.rdbuf();
    init(sstr.str());
    this->path = path;
}

void
AnyFile::init(const u8 *buf, isize len)
{    
    assert(buf);
    if (!isCompatibleBuffer(buf, len)) throw AppError(Fault::FILE_TYPE_MISMATCH);
    readFromBuffer(buf, len);
}

AnyFile::~AnyFile()
{

}

void
AnyFile::flash(u8 *buf, isize offset, isize len) const
{
    assert(buf);
    std::memcpy(buf + offset, data.ptr, len);
}

void
AnyFile::flash(u8 *buf, isize offset) const
{
    flash (buf, offset, data.size);
}

bool
AnyFile::isCompatibleBuffer(const Buffer<u8> &buffer)
{
    return isCompatibleBuffer(buffer.ptr, buffer.size);
}

isize
AnyFile::readFromBuffer(const u8 *buf, isize len)
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
AnyFile::readFromBuffer(const Buffer<u8> &buffer)
{
    return readFromBuffer(buffer.ptr, buffer.size);
}

isize
AnyFile::writeToStream(std::ostream &stream, isize offset, isize len) const
{
    assert(offset >= 0 && len >= 0 && offset + len <= data.size);

    stream.write((char *)data.ptr + offset, len);
    
    return data.size;
}

isize
AnyFile::writeToFile(const fs::path &path, isize offset, isize len) const
{
    if (util::isDirectory(path)) {
        throw AppError(Fault::FILE_IS_DIRECTORY);
    }
    
    std::ofstream stream(path, std::ofstream::binary);

    if (!stream.is_open()) {
        throw AppError(Fault::FILE_CANT_WRITE, path);
    }
    
    isize result = writeToStream(stream, offset, len);
    assert(result == data.size);
    
    return result;
}

isize
AnyFile::writeToBuffer(u8 *buf, isize offset, isize len) const
{
    assert(buf);
    assert(offset >= 0 && offset < data.size);
    assert(len >= 0 && offset + len <= data.size);

    std::memcpy(buf, (char *)data.ptr + offset, len);

    return data.size;
}

isize
AnyFile::writeToBuffer(Buffer<u8> &buffer, isize offset, isize len) const
{
    buffer.alloc(len);
    return writeToBuffer(buffer.ptr, offset, len);
}

isize
AnyFile::writeToStream(std::ostream &stream) const
{
    return writeToStream(stream, 0, data.size);
}

isize
AnyFile::writeToFile(const fs::path &path) const
{
    return writeToFile(path, 0, data.size);
}

isize 
AnyFile::writePartitionToFile(const fs::path &path, isize partition) const
{
    throw AppError(Fault::FILE_TYPE_UNSUPPORTED);
}

isize
AnyFile::writeToBuffer(u8 *buf) const
{
    return writeToBuffer(buf, 0, data.size);
}

isize
AnyFile::writeToBuffer(Buffer<u8> &buffer) const
{
    return writeToBuffer(buffer, 0, data.size);
}

}
