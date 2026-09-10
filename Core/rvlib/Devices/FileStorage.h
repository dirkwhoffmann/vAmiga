// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Devices/BlockStorage.h"
#include <array>
#include <fstream>
#include <list>
#include <map>

namespace retro::vault {

/* Storage that leaves the image on disk and reads it where it is looked at.
 *
 * Built for images too large to hold in memory. The device is divided into
 * pages; a page is read from the file the first time it is touched and kept
 * in a cache of bounded size, so a working set stays in memory while the rest
 * of a multi-gigabyte image never occupies any.
 *
 * Writes never reach the file. A written page moves out of the cache into an
 * overlay that is not subject to eviction, and every later read sees the
 * overlay's copy. The backing file is therefore read-only for the lifetime of
 * this object, which is what makes the whole arrangement safe:
 *
 * - The emulator's time travel keeps working. Rewinding to a snapshot does
 *   not have to undo anything on disk, because nothing was done to it.
 * - A crash cannot corrupt the user's image.
 * - Clones share the backing file. The run-ahead instance is a clone that
 *   runs ahead and writes as it goes; because those writes land in its own
 *   overlay, they cannot reach either the file or the instance it came from.
 *
 * The cost is that the overlay grows with everything the guest writes, and
 * nothing shrinks it short of flush(). A guest that rewrites the entire disk
 * ends up holding the entire disk -- no worse than RamStorage, which holds it
 * from the outset, but no better either. Spilling cold overlay pages to a
 * scratch file is the obvious next move if that ever bites.
 *
 * Not thread-safe, and deliberately so: reads mutate the cache, and clones
 * share one file handle. Every user of this lives on the emulator thread.
 */
class FileStorage final : public BlockStorage {

public:

    // Bytes per page
    static constexpr isize psize = 4096;

    // Default number of clean pages held in the cache (16 MB)
    static constexpr isize defaultCacheSize = 4096;

private:

    /* The image on disk.
     *
     * Held by shared_ptr because clones read through the same handle. Nothing
     * ever writes to it, so there is nothing for them to disagree about.
     */
    struct Backing {

        fs::path path;
        mutable std::ifstream stream;
        isize bytes = 0;
    };

    std::shared_ptr<Backing> backing;

    // Pages that have been written. Never evicted
    std::map<isize, std::array<u8, psize>> overlay;

    // Pages read from the file, evicted in least-recently-used order
    struct CachedPage {

        std::array<u8, psize> data;
        std::list<isize>::iterator pos;
    };

    mutable std::map<isize, CachedPage> cache;
    mutable std::list<isize> used;
    isize cacheSize = defaultCacheSize;


    //
    // Initializing
    //

public:

    /* Opens an image for reading. Throws if it cannot be read.
     *
     * 'bytes' is how much of the file this storage presents; 0 means all of
     * it. An image may hold more than the geometry it declares accounts for,
     * and a device has to be exactly as large as its geometry says.
     */
    explicit FileStorage(const fs::path &path, isize bytes = 0,
                         isize cacheSize = defaultCacheSize);

    std::unique_ptr<BlockStorage> clone() const override;


    //
    // Methods from BlockStorage
    //

public:

    isize size() const override { return backing->bytes; }

    /* Not supported.
     *
     * Resizing would mean abandoning the image this storage exists to serve.
     * A caller that wants a differently sized drive wants a different
     * storage, and should install one.
     */
    void alloc(isize bytes, u8 value = 0) override;
    void dealloc() override;

    void read(u8 *dst, isize offset, isize count) const override;
    void write(const u8 *src, isize offset, isize count) override;
    void clear(u8 value, isize offset, isize count) override;

    fs::path backingPath() const override { return backing->path; }
    isize pendingBytes() const override { return isize(overlay.size()) * psize; }
    void flush() override;


    //
    // Managing pages
    //

private:

    // Returns a page for reading, faulting it in if necessary
    const u8 *readPage(isize nr) const;

    // Returns a page for writing, moving it into the overlay
    u8 *writePage(isize nr);

    // Returns a clean page, reading it from the file if necessary
    const u8 *cachedPage(isize nr) const;

    // Removes a page from the cache
    void uncache(isize nr) const;

    // Applies 'body' to every page overlapping a byte range
    template <typename T>
    void eachPage(isize offset, isize count, T body) const;
};

}
