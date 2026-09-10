// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "utl/storage/FileBackedBuffer.h"
#include "utl/io/IOError.h"
#include <atomic>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace utl {

namespace {

// Builds a name no other buffer in this process will pick
fs::path uniqueTempPath()
{
    static std::atomic<u64> counter { 0 };

    for (isize attempt = 0; attempt < 1000; attempt++) {

        auto name = "utl-" + std::to_string(u64(::getpid())) +
                    "-" + std::to_string(counter++) + ".tmp";

        auto candidate = fs::temp_directory_path() / name;
        if (!fs::exists(candidate)) return candidate;
    }

    throw IOError(IOError::FILE_CANT_CREATE, fs::temp_directory_path());
}

}

FileBackedBuffer::FileBackedBuffer(isize size, bool writeThrough, bool readOnly)
{
    alloc(size, writeThrough, readOnly);
}

FileBackedBuffer::FileBackedBuffer(const fs::path &path, bool writeThrough, bool readOnly)
{
    alloc(path, writeThrough, readOnly);
}

FileBackedBuffer::~FileBackedBuffer()
{
    dealloc();
}

FileBackedBuffer::FileBackedBuffer(FileBackedBuffer &&other) noexcept
{
    *this = std::move(other);
}

FileBackedBuffer &
FileBackedBuffer::operator=(FileBackedBuffer &&other) noexcept
{
    if (this == &other) return *this;

    dealloc();

    url = std::move(other.url);
    addr = other.addr;
    bytes = other.bytes;
    wt = other.wt;
    ro = other.ro;
    owned = other.owned;
    fileHandle = other.fileHandle;
    mapHandle = other.mapHandle;
    fd = other.fd;

    // The source must not release what it no longer holds
    other.url.clear();
    other.addr = nullptr;
    other.bytes = 0;
    other.wt = false;
    other.ro = false;
    other.owned = false;
    other.fileHandle = nullptr;
    other.mapHandle = nullptr;
    other.fd = -1;

    return *this;
}

void
FileBackedBuffer::alloc(isize size, bool writeThrough, bool readOnly)
{
    dealloc();

    wt = writeThrough;
    ro = readOnly;

    if (size < 0) throw IOError(IOError::FILE_CANT_CREATE, "negative size");
    if (size == 0) return;

    auto tmp = uniqueTempPath();

    // Create the file and grow it to the requested length
    {
        std::ofstream out(tmp, std::ios::binary);
        if (!out) throw IOError(IOError::FILE_CANT_CREATE, tmp);
    }

    std::error_code ec;
    fs::resize_file(tmp, uintmax_t(size), ec);

    if (ec) {

        fs::remove(tmp, ec);
        throw IOError(IOError::FILE_CANT_CREATE, tmp);
    }

    url = tmp;
    bytes = size;

    /* Claim ownership before mapping, so that a failure below still takes the
     * file with it rather than leaving it behind.
     */
    owned = true;

    try { map(); } catch (...) { dealloc(); throw; }
}

void
FileBackedBuffer::alloc(const fs::path &path, bool writeThrough, bool readOnly)
{
    dealloc();

    wt = writeThrough;
    ro = readOnly;

    std::error_code ec;
    auto len = fs::file_size(path, ec);

    if (ec) throw IOError(IOError::FILE_NOT_FOUND, path);

    url = path;
    bytes = isize(len);
    owned = false;

    // An empty file is an empty buffer, not an error
    if (bytes == 0) return;

    try { map(); } catch (...) { dealloc(); throw; }
}

void
FileBackedBuffer::map()
{
    /* Decide how the file can be opened before deciding how to map it.
     *
     * A shared mapping needs write access; a private one does not, and stays
     * usable over a file we may only read.
     */
#ifdef _WIN32

    auto open = [&](bool forWriting) {

        return ::CreateFileW(url.wstring().c_str(),
                             forWriting ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
                             FILE_SHARE_READ,
                             nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    };

    auto handle = open(!ro);

    if (handle == INVALID_HANDLE_VALUE) {

        // The caller asked for writes to reach the file, and they cannot
        if (shared()) throw IOError(IOError::FILE_CANT_WRITE, url);

        handle = open(false);
        if (handle == INVALID_HANDLE_VALUE) throw IOError(IOError::FILE_CANT_READ, url);

        // Not asked for, but true all the same
        ro = true;
    }

    fileHandle = handle;

    mapHandle = ::CreateFileMappingW((HANDLE)fileHandle, nullptr,
                                     ro ? PAGE_READONLY : PAGE_READWRITE,
                                     0, 0, nullptr);

    if (!mapHandle) throw IOError(IOError::FILE_CANT_READ, url);

    auto access = shared() ? FILE_MAP_WRITE : FILE_MAP_COPY;
    addr = (u8 *)::MapViewOfFile((HANDLE)mapHandle, access, 0, 0, 0);

    if (!addr) throw IOError(IOError::FILE_CANT_READ, url);

#else

    fd = ro ? -1 : ::open(url.c_str(), O_RDWR);

    if (fd < 0) {

        // The caller asked for writes to reach the file, and they cannot
        if (shared()) throw IOError(IOError::FILE_CANT_WRITE, url);

        fd = ::open(url.c_str(), O_RDONLY);
        if (fd < 0) throw IOError(IOError::FILE_CANT_READ, url);

        // Not asked for, but true all the same
        ro = true;
    }

    /* PROT_WRITE is requested either way.
     *
     * With MAP_PRIVATE it means "these pages may be modified", not "this file
     * may be modified" -- the copy on write happens in our own memory, so it
     * is legitimate even over a file opened read only.
     */
    auto flags = shared() ? MAP_SHARED : MAP_PRIVATE;
    auto *p = ::mmap(nullptr, size_t(bytes), PROT_READ | PROT_WRITE, flags, fd, 0);

    if (p == MAP_FAILED) {
        throw IOError(shared() ? IOError::FILE_CANT_WRITE : IOError::FILE_CANT_READ, url);
    }

    addr = (u8 *)p;

#endif
}

void
FileBackedBuffer::remap(bool wasShared)
{
    /* Leaving a private mapping for a shared one drops whatever was written
     * privately -- those pages are ours, not the file's, and the new mapping
     * will read the file. Put them there first.
     */
    if (!wasShared && shared()) writeBack();

    // Keep hold of the old mapping until the new one stands
    auto *oldAddr = addr;
    auto oldFileHandle = fileHandle;
    auto oldMapHandle = mapHandle;
    auto oldFd = fd;

    addr = nullptr;
    fileHandle = nullptr;
    mapHandle = nullptr;
    fd = -1;

    try {

        map();

    } catch (...) {

        // Put the old mapping back, so a failed switch changes nothing
        addr = oldAddr;
        fileHandle = oldFileHandle;
        mapHandle = oldMapHandle;
        fd = oldFd;
        throw;
    }

#ifdef _WIN32

    if (oldAddr) ::UnmapViewOfFile(oldAddr);
    if (oldMapHandle) ::CloseHandle((HANDLE)oldMapHandle);
    if (oldFileHandle && oldFileHandle != INVALID_HANDLE_VALUE) ::CloseHandle((HANDLE)oldFileHandle);

#else

    if (oldAddr) ::munmap(oldAddr, size_t(bytes));
    if (oldFd >= 0) ::close(oldFd);

#endif
}

void
FileBackedBuffer::setWriteThrough(bool value)
{
    if (wt == value) return;

    auto wasShared = shared();
    wt = value;

    if (empty() || shared() == wasShared) return;

    try { remap(wasShared); } catch (...) { wt = !value; throw; }
}

void
FileBackedBuffer::setReadOnly(bool value)
{
    if (ro == value) return;

    auto wasShared = shared();
    ro = value;

    if (empty() || shared() == wasShared) return;

    try { remap(wasShared); } catch (...) { ro = !value; throw; }
}

void
FileBackedBuffer::dealloc()
{
#ifdef _WIN32

    if (addr) ::UnmapViewOfFile(addr);
    if (mapHandle) ::CloseHandle((HANDLE)mapHandle);
    if (fileHandle && fileHandle != INVALID_HANDLE_VALUE) ::CloseHandle((HANDLE)fileHandle);

#else

    if (addr) ::munmap(addr, size_t(bytes));
    if (fd >= 0) ::close(fd);

#endif

    // A temporary file has no reason to outlive the buffer it was made for
    if (owned && !url.empty()) {

        std::error_code ec;
        fs::remove(url, ec);
    }

    url.clear();
    addr = nullptr;
    bytes = 0;
    owned = false;

    /* Both modes describe the file we just let go of, so they go with it.
     * The next alloc() states them afresh.
     */
    wt = false;
    ro = false;
    fileHandle = nullptr;
    mapHandle = nullptr;
    fd = -1;
}

void
FileBackedBuffer::flush()
{
    // The file is already being written, or must not be
    if (wt || ro) return;

    writeBack();
}

void
FileBackedBuffer::writeBack()
{
    if (!addr || url.empty()) return;

    /* Write the buffer back in full.
     *
     * A private mapping keeps no record of which pages were modified, so
     * there is no smaller set of bytes to write. Writing what the mapping
     * already contains is safe even though the file is mapped: any page still
     * shared with the page cache receives exactly the bytes it already held.
     */
    std::ofstream out(url, std::ios::binary | std::ios::in | std::ios::out);

    if (!out.is_open()) throw IOError(IOError::FILE_CANT_WRITE, url);

    out.write((const char *)addr, bytes);
    out.flush();

    if (!out) throw IOError(IOError::FILE_CANT_WRITE, url);
}

}
