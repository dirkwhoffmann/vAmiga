// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "rvconfig.h"
#include "Devices/FileStorage.h"
#include "utl/io.h"

namespace retro::vault {

using utl::IOError;

FileStorage::FileStorage(const fs::path &path, isize bytes, isize cacheSize) :
cacheSize(cacheSize)
{
    assert(cacheSize > 0);

    backing = std::make_shared<Backing>();
    backing->path = path;

    std::error_code ec;
    auto onDisk = isize(fs::file_size(path, ec));
    if (ec) throw IOError(IOError::FILE_NOT_FOUND, path);

    if (bytes < 0 || bytes > onDisk) throw IOError(IOError::FILE_CANT_READ, path);

    backing->bytes = bytes ? bytes : onDisk;
    backing->stream.open(path, std::ios::binary);

    if (!backing->stream.is_open()) {
        throw IOError(IOError::FILE_CANT_READ, path);
    }
}

std::unique_ptr<BlockStorage>
FileStorage::clone() const
{
    /* The clone reads through the same file handle and starts out with a copy
     * of this instance's overlay. It deliberately does not inherit the cache:
     * the cache is pure duplication of what the file already holds, and the
     * clone will fault back in whatever it actually touches.
     */
    auto result = std::unique_ptr<FileStorage>(new FileStorage(*this));
    result->cache.clear();
    result->used.clear();

    return result;
}

void
FileStorage::alloc(isize bytes, u8 value)
{
    throw IOError(IOError::FILE_CANT_WRITE, backing->path);
}

void
FileStorage::dealloc()
{
    // Drop everything that is not the image itself
    overlay.clear();
    cache.clear();
    used.clear();
}

const u8 *
FileStorage::cachedPage(isize nr) const
{
    // Serve the page from the cache if it is there
    if (auto it = cache.find(nr); it != cache.end()) {

        used.splice(used.begin(), used, it->second.pos);
        return it->second.data.data();
    }

    // Make room before inserting, so the new page cannot be the one evicted
    while (isize(cache.size()) >= cacheSize) uncache(used.back());

    CachedPage page = {};

    auto offset = nr * psize;
    auto count = std::min(psize, backing->bytes - offset);

    if (count > 0) {

        backing->stream.seekg(offset, std::ios::beg);
        backing->stream.read((char *)page.data.data(), count);

        if (!backing->stream) {
            backing->stream.clear();
            throw IOError(IOError::FILE_CANT_READ, backing->path);
        }
    }

    used.push_front(nr);
    auto &entry = cache.emplace(nr, page).first->second;
    entry.pos = used.begin();

    return entry.data.data();
}

void
FileStorage::uncache(isize nr) const
{
    if (auto it = cache.find(nr); it != cache.end()) {

        used.erase(it->second.pos);
        cache.erase(it);
    }
}

const u8 *
FileStorage::readPage(isize nr) const
{
    if (auto it = overlay.find(nr); it != overlay.end()) return it->second.data();

    return cachedPage(nr);
}

u8 *
FileStorage::writePage(isize nr)
{
    if (auto it = overlay.find(nr); it != overlay.end()) return it->second.data();

    // Move the page into the overlay, where eviction cannot reach it
    std::array<u8, psize> page;
    std::memcpy(page.data(), cachedPage(nr), psize);
    uncache(nr);

    return overlay.emplace(nr, page).first->second.data();
}

template <typename T> void
FileStorage::eachPage(isize offset, isize count, T body) const
{
    assert(offset >= 0 && count >= 0 && offset + count <= size());

    while (count > 0) {

        auto nr = offset / psize;
        auto within = offset % psize;
        auto len = std::min(count, psize - within);

        body(nr, within, len);

        offset += len;
        count -= len;
    }
}

void
FileStorage::read(u8 *dst, isize offset, isize count) const
{
    eachPage(offset, count, [&](isize nr, isize within, isize len) {

        std::memcpy(dst, readPage(nr) + within, size_t(len));
        dst += len;
    });
}

void
FileStorage::write(const u8 *src, isize offset, isize count)
{
    eachPage(offset, count, [&](isize nr, isize within, isize len) {

        std::memcpy(writePage(nr) + within, src, size_t(len));
        src += len;
    });
}

void
FileStorage::clear(u8 value, isize offset, isize count)
{
    eachPage(offset, count, [&](isize nr, isize within, isize len) {

        std::memset(writePage(nr) + within, value, size_t(len));
    });
}

void
FileStorage::flush()
{
    if (overlay.empty()) return;

    std::fstream out(backing->path, std::ios::binary | std::ios::in | std::ios::out);

    if (!out.is_open()) {
        throw IOError(IOError::FILE_CANT_WRITE, backing->path);
    }

    for (const auto &[nr, page] : overlay) {

        auto offset = nr * psize;
        auto count = std::min(psize, backing->bytes - offset);

        out.seekp(offset, std::ios::beg);
        out.write((const char *)page.data(), count);

        if (!out) throw IOError(IOError::FILE_CANT_WRITE, backing->path);
    }

    out.flush();
    out.close();

    /* The written pages are now what the file holds, so they belong in the
     * cache rather than the overlay. Simply dropping them is correct and
     * cheaper to reason about; they fault back in when next touched.
     */
    overlay.clear();
    cache.clear();
    used.clear();
}

}
