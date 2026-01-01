// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AnyImage.h"
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
#include "utl/io.h"
#include "utl/support.h"
#include <fstream>

namespace vamiga {

optional<ImageInfo>
AnyImage::about(const fs::path& url)
{
    if (auto info = DiskImage::about(url)) return info;
    return {};
}

void
AnyImage::init(isize len)
{
    data.init(len);
}

void
AnyImage::init(const Buffer<u8> &buffer)
{
    init(buffer.ptr, buffer.size);
}

void
AnyImage::init(const string &str)
{
    init((const u8 *)str.c_str(), (isize)str.length());
}

void
AnyImage::init(const fs::path &path)
{
    std::ifstream stream(path, std::ios::binary);

    if (!stream.is_open()) {
        throw IOError(IOError::FILE_NOT_FOUND, path);
    }
    if (!validateURL(path)) {
        throw IOError(IOError::FILE_TYPE_MISMATCH, path);
    }
    std::ostringstream sstr(std::ios::binary);
    sstr << stream.rdbuf();
    init(sstr.str());
    this->path = path;
}

void
AnyImage::init(const u8 *buf, isize len)
{
    assert(buf);

    // Allocate memory
    data.alloc(len);

    // Copy data
    std::memcpy(data.ptr, buf, data.size);
    didLoad();
}

void
AnyImage::copy(u8 *buf, isize offset, isize len) const
{
    assert(buf);
    assert(offset >= 0 && offset < data.size);
    assert(len >= 0 && offset + len <= data.size);

    std::memcpy(buf + offset, data.ptr, len);
}

ByteView
AnyImage::byteView(isize offset) const
{
    return byteView(offset, data.size - offset);
}

ByteView
AnyImage::byteView(isize offset, isize len) const
{
    assert(offset >= 0 && offset < data.size);
    assert(len >= 0 && offset + len <= data.size);

    return ByteView(data.ptr + offset, len);
}

MutableByteView
AnyImage::byteView(isize offset)
{
    return byteView(offset, data.size - offset);
}

MutableByteView
AnyImage::byteView(isize offset, isize len)
{
    assert(offset >= 0 && offset < data.size);
    assert(len >= 0 && offset + len <= data.size);

    return MutableByteView(data.ptr + offset, len);
}

void
AnyImage::copy(u8 *buf, isize offset) const
{
    copy (buf, offset, data.size);
}

isize
AnyImage::writeToStream(std::ostream &stream, isize offset, isize len) const
{
    assert(offset >= 0 && len >= 0 && offset + len <= data.size);

    stream.write((char *)data.ptr + offset, len);

    return data.size;
}

isize
AnyImage::writeToFile(const fs::path &path, isize offset, isize len) const
{
    if (utl::isDirectory(path)) {
        throw IOError(IOError::FILE_IS_DIRECTORY);
    }

    std::ofstream stream(path, std::ofstream::binary);

    if (!stream.is_open()) {
        throw IOError(IOError::FILE_CANT_WRITE, path);
    }

    isize result = writeToStream(stream, offset, len);
    assert(result == data.size);

    return result;
}

isize
AnyImage::writeToStream(std::ostream &stream) const
{
    return writeToStream(stream, 0, data.size);
}

isize
AnyImage::writeToFile(const fs::path &path) const
{
    return writeToFile(path, 0, data.size);
}

}
