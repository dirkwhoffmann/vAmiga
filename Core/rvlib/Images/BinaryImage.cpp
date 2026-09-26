// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "rvconfig.h"
#include "Images/BinaryImage.h"
#include "Devices/LinearDevice.h"
#include "utl/abilities/Compressible.h"
#include "utl/io.h"
#include "utl/storage/Buffer.h"
#include "utl/support.h"
#include <fstream>

namespace retro::vault {

using utl::ByteView;
using utl::MutableByteView;
using utl::IOError;

void
BinaryImage::init(isize len)
{
    data.init(len);
}

void
BinaryImage::init(const fs::path &p)
{
    if (!validateURL(p))
        throw utl::IOError(utl::IOError::FILE_TYPE_MISMATCH, p);

    /* A compressed file is unpacked first, and the image is put on top of the
     * result. Everything below this line therefore deals with plain bytes,
     * whichever of the two the image was asked for.
     */
    if (isPacked(p)) unpacked = unpack(p);
    const auto &source = isPacked(p) ? unpacked.path() : p;

    if (utl::getSizeOfFile(source) <= 0)
        throw utl::IOError(utl::IOError::FILE_CANT_READ, p);

    // Ask the format how large the image in that file is
    auto size = imageSize(source);

    this->path = p;

    // Put the image on top of the file. Nothing is loaded yet.
    data.init(size, source);
    didInitialize();
}

void
BinaryImage::init(const LinearDevice &device)
{
    data.init(device.size());

    // Pull in the contents
    auto bytes = data.mutableByteView(0, data.size());
    device.read(bytes.data(), 0, bytes.size());
    didInitialize();
}

void
BinaryImage::init(const u8 *buf, isize len)
{
    assert(buf);

    data.init(len);

    if (len) std::memcpy(data.mutableByteView(0, len).data(), buf, size_t(len));
    didInitialize();
}

isize
BinaryImage::imageSize(const fs::path &p) const
{
    return utl::getSizeOfFile(p);
}

bool
BinaryImage::isPacked(const fs::path &p)
{
    auto suffix = utl::lowercased(p.extension().string());

    return suffix == ".adz" || suffix == ".hdz";
}

utl::TempFile
BinaryImage::unpack(const fs::path &packed)
{
    if (!fs::exists(packed))
        throw utl::IOError(utl::IOError::FILE_NOT_FOUND, packed);

    utl::Buffer<u8> data;
    data.init(packed);

    if (data.empty())
        throw utl::IOError(utl::IOError::FILE_CANT_READ, packed);

    try {
        data.gunzip();
    } catch (std::exception &err) {
        throw utl::IOError(utl::IOError::ZLIB_ERROR, err.what());
    }

    // Unpack into a file of the format the packed one holds (.adz -> .adf)
    utl::TempFile temp(utl::lowercased(packed.extension().string()) == ".adz" ?
                       ".adf" : ".hdf");
    data.write(temp.path());

    return temp;
}

void
BinaryImage::pack(const fs::path &plain, const fs::path &packed)
{
    utl::Buffer<u8> data;
    data.init(plain);

    // Compress first, so that a failure here leaves the target alone
    try {
        data.gzip();
    } catch (std::exception &err) {
        throw utl::IOError(utl::IOError::ZLIB_ERROR, err.what());
    }

    data.write(packed);
}

utl::ByteView
BinaryImage::byteView(isize offset, isize len) const
{
    return data.byteView(offset, len);
}

utl::MutableByteView
BinaryImage::mutableByteView(isize offset, isize len)
{
    return data.mutableByteView(offset, len);
}

void
BinaryImage::detach()
{
    data.detach();
    path.clear();
}

void
BinaryImage::copy(u8 *buf, isize offset, isize len) const
{
    assert(buf);

    std::memcpy(buf, byteView(offset, len).data(), len);
}

void
BinaryImage::copy(u8 *buf, isize offset) const
{
    copy(buf, offset, getSize() - offset);
}

void
BinaryImage::save()
{
    // An image built in memory has nowhere to persist to yet
    if (!data.backed()) { saveAs(path); return; }

    auto modified = data.dirty();

    // Write the modified parts back to where the image came from
    data.persist();

    /* An image from a packed file has just written into the temporary file
     * it was unpacked into. The packed file is rewritten from that, in full,
     * because a compressed file cannot be updated in pieces.
     */
    if (modified && !unpacked.empty()) pack(unpacked.path(), path);
}

void
BinaryImage::saveAs(const fs::path &newPath)
{
    auto size = getSize();

    if (isPacked(newPath)) {

        /* Write the plain bytes to a temporary file, pack that into the new
         * file, and go on living on the temporary one. Unpacking what was
         * just written would give the same thing at twice the price.
         */
        utl::TempFile temp(".tmp");
        writeToFile(temp.path());
        pack(temp.path(), newPath);

        path = newPath;
        data.init(size, temp.path());
        unpacked = std::move(temp);
        return;
    }

    // Write the entire image first, so that a failure changes nothing
    writeToFile(newPath);

    // Continue on top of the new file, which now holds exactly this image
    path = newPath;
    data.init(size, newPath);
    unpacked.reset();
}

isize
BinaryImage::writeToStream(std::ostream &stream, isize offset, isize len) const
{
    stream.write((const char *)byteView(offset, len).data(), len);

    return len;
}

isize
BinaryImage::writeToFile(const fs::path &p, isize offset, isize len) const
{
    if (utl::isDirectory(p)) {
        throw utl::IOError(utl::IOError::FILE_IS_DIRECTORY);
    }

    /* A packed target is written in two steps: the plain bytes go to a
     * temporary file, which is then packed into the target. The byte count
     * is the one of the file that was asked for, so it counts packed bytes.
     */
    if (isPacked(p)) {

        utl::TempFile temp(".tmp");
        writeToFile(temp.path(), offset, len);
        pack(temp.path(), p);

        return isize(fs::file_size(p));
    }

    /* The target may be the very file this image is loaded from. Opening it
     * for writing truncates it, so whatever is still in there only has to
     * come in first.
     */
    std::error_code ec;
    if (fs::equivalent(p, path, ec)) (void)byteView(0, getSize());

    std::ofstream stream(p, std::ofstream::binary);

    if (!stream.is_open()) {
        throw utl::IOError(utl::IOError::FILE_CANT_WRITE, p);
    }

    isize result = writeToStream(stream, offset, len);
    assert(result == len);

    return result;
}

isize
BinaryImage::writeToStream(std::ostream &stream) const
{
    return writeToStream(stream, 0, getSize());
}

isize
BinaryImage::writeToFile(const fs::path &p) const
{
    return writeToFile(p, 0, getSize());
}

}
