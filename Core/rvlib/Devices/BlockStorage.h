// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Devices/DeviceTypes.h"
#include "utl/common.h"
#include "utl/storage.h"
#include <cstring>
#include <memory>

namespace retro::vault {

/* The bytes behind a block device.
 *
 * Separates *where the data lives* from *what the data means*. A device such
 * as a hard drive owns one of these and asks it for bytes; whether those bytes
 * come from a buffer in RAM, from a file on disk, or from a cache in front of
 * a file is the storage's business alone.
 *
 * The interface is deliberately small. It is the whole of what a device needs
 * in order to stop caring where its data lives: a size, byte-granular reads
 * and writes, and a way to be allocated and thrown away. Everything that has
 * a *meaning* -- geometry, partitions, block numbers, file systems -- stays
 * with the device, because none of it changes when the backing store does.
 *
 * The one concession is buffer(). Some call sites still need a contiguous
 * pointer into the image, and rather than let them reach around the interface
 * silently, this makes the requirement explicit: a storage that cannot hand
 * out such a pointer returns nullptr, and the caller is forced to notice.
 */
class BlockStorage {

public:

    virtual ~BlockStorage() = default;

    // Creates an independent copy of this storage
    virtual std::unique_ptr<BlockStorage> clone() const = 0;


    //
    // Querying the capacity
    //

public:

    // Total capacity in bytes
    virtual isize size() const = 0;

    // Returns true if no space has been allocated
    bool empty() const { return size() == 0; }


    //
    // Allocating
    //

public:

    // Discards all contents and allocates space for the given number of bytes
    virtual void alloc(isize bytes, u8 value = 0) = 0;

    // Releases all space
    virtual void dealloc() = 0;


    //
    // Reading and writing
    //

public:

    // Transfers a sequence of bytes starting at the given byte offset
    virtual void read(u8 *dst, isize offset, isize count) const = 0;
    virtual void write(const u8 *src, isize offset, isize count) = 0;

    // Overwrites a sequence of bytes with a constant value
    virtual void clear(u8 value, isize offset, isize count) = 0;

    // Transfers a single byte
    u8 readByte(isize offset) const { u8 v; read(&v, offset, 1); return v; }
    void writeByte(isize offset, u8 value) { write(&value, offset, 1); }


    //
    // Accessing the raw contents
    //

public:

    /* Returns the backing buffer, or nullptr if there is none.
     *
     * Only storage that keeps the whole image in one contiguous allocation
     * can answer this. Callers that depend on it are, by definition, callers
     * that cannot work with a lazily loaded image.
     */
    virtual utl::Buffer<u8> *buffer() { return nullptr; }
    virtual const utl::Buffer<u8> *buffer() const { return nullptr; }


    //
    // Persisting
    //

public:

    // Returns the file this storage reads from, if it has one
    virtual fs::path backingPath() const { return {}; }

    // Number of bytes written but not yet persisted
    virtual isize pendingBytes() const { return 0; }

    /* Writes pending changes back to the backing file.
     *
     * A no-op for storage that has no file behind it -- there is nowhere for
     * the changes to go, and the caller is expected to export the contents
     * instead.
     */
    virtual void flush() { }
};


/* Storage that keeps the entire image in RAM.
 *
 * The classic scheme: one allocation, held for as long as the device exists.
 * Reads and writes are plain memcpys, which is as fast as this gets, and the
 * whole image is available as a pointer. The price is the obvious one -- the
 * image has to fit in memory, twice over if the device is cloned.
 */
class RamStorage final : public BlockStorage {

    // The raw data of this device
    utl::Buffer<u8> data;

public:

    RamStorage() = default;
    explicit RamStorage(isize bytes, u8 value = 0) { alloc(bytes, value); }
    RamStorage(const RamStorage &other) { data.init(other.data); }

    std::unique_ptr<BlockStorage> clone() const override {
        return std::make_unique<RamStorage>(*this);
    }


    //
    // Methods from BlockStorage
    //

public:

    isize size() const override { return data.size; }

    void alloc(isize bytes, u8 value = 0) override { data.init(bytes, value); }
    void dealloc() override { data.dealloc(); }

    void read(u8 *dst, isize offset, isize count) const override {

        assert(offset >= 0 && count >= 0 && offset + count <= data.size);
        if (count) std::memcpy(dst, data.ptr + offset, size_t(count));
    }

    void write(const u8 *src, isize offset, isize count) override {

        assert(offset >= 0 && count >= 0 && offset + count <= data.size);
        if (count) std::memcpy(data.ptr + offset, src, size_t(count));
    }

    void clear(u8 value, isize offset, isize count) override {

        assert(offset >= 0 && count >= 0 && offset + count <= data.size);
        if (count) std::memset(data.ptr + offset, value, size_t(count));
    }

    utl::Buffer<u8> *buffer() override { return &data; }
    const utl::Buffer<u8> *buffer() const override { return &data; }
};

}
