// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/common.h"

namespace utl {

/* A byte buffer whose storage is a file, mapped into memory.
 *
 * It behaves like an ordinary buffer -- raw() hands out a pointer, and the
 * bytes behind it are read and written like any other memory -- but the
 * memory is a window onto a file. The operating system reads pages in as they
 * are touched and evicts them again under pressure, so a buffer far larger
 * than RAM costs only what is actually used.
 *
 * Two flags decide what a write means, and they are fixed when the buffer is
 * allocated:
 *
 *   writeThrough  readOnly   a write to raw() ...        flush() ...
 *   ----------------------------------------------------------------------
 *   true          false      goes to the file at once    does nothing
 *   false         false      stays private to us         writes the file
 *   false         true       stays private to us         does nothing
 *   true          true       -- rejected by alloc() --
 *
 * readOnly is not requested, it is observed: it is set when the file could
 * not be opened for writing. Asking for writeThrough on such a file throws,
 * because the caller asked for writes to reach the file and they cannot.
 *
 * Where the file comes from:
 *
 * - alloc(path) uses an existing file and leaves it behind when the buffer
 *   goes away.
 * - alloc(size) creates a temporary file of that length and deletes it again
 *   with the buffer. path() names it, so it can be found while it lives.
 *
 * A buffer of size zero has no file and no mapping: raw() is nullptr. That is
 * also the state of a default constructed buffer, and the state after
 * dealloc().
 *
 * Two properties worth knowing before reaching for this:
 *
 * - An I/O error, or the file being truncated underneath the mapping, raises
 *   a signal rather than an exception, and cannot be caught the way a failed
 *   read() can. On local storage that is rare; on a network volume or
 *   removable media it is not.
 * - flush() rewrites the file in full. Private modifications cannot be
 *   located after the fact, so there is nothing smaller to write.
 */
class FileBackedBuffer {

public:

    /* Whether writes reach the file as they happen. An input.
     *
     * Read by alloc(), and only by alloc(). Setting it on a buffer that is
     * already allocated changes nothing -- how a mapping treats writes is
     * fixed when the mapping is made. Set it first, then allocate. It
     * survives dealloc(), so a buffer can be reallocated without setting it
     * again.
     */
    bool writeThrough = false;

    /* Whether the file could not be opened for writing. An output.
     *
     * Set by alloc(), never by the caller. Cleared by dealloc().
     */
    bool readOnly = false;

private:

    // The file behind this buffer (empty if there is none)
    fs::path url;

    // The mapped region
    u8 *addr = nullptr;
    isize bytes = 0;

    // Set when the file is ours, and dies with us
    bool owned = false;

    // Platform handles
    void *fileHandle = nullptr;
    void *mapHandle = nullptr;
    int fd = -1;


    //
    // Initializing
    //

public:

    // Creates an empty buffer
    FileBackedBuffer() = default;

    // Creates a buffer of the given length, backed by a temporary file
    FileBackedBuffer(isize size, bool writeThrough);

    // Creates a buffer backed by an existing file
    FileBackedBuffer(const fs::path &path, bool writeThrough);

    ~FileBackedBuffer();

    /* A mapping is a resource, not a value.
     *
     * Copying is refused rather than given some invented meaning: two buffers
     * over one file would disagree about who owns it, and about whether a
     * write in one is visible in the other. Moving is what callers want.
     */
    FileBackedBuffer(const FileBackedBuffer &) = delete;
    FileBackedBuffer &operator=(const FileBackedBuffer &) = delete;

    FileBackedBuffer(FileBackedBuffer &&other) noexcept;
    FileBackedBuffer &operator=(FileBackedBuffer &&other) noexcept;


    //
    // Allocating
    //

public:

    /* Backs the buffer with a temporary file of the given length.
     *
     * The file is created empty and grown to size, which leaves it sparse
     * where the file system supports that, and it is deleted when the buffer
     * is. A size of zero allocates nothing.
     */
    void alloc(isize size);

    /* Backs the buffer with an existing file, in its entirety.
     *
     * The file outlives the buffer. Throws if it does not exist, or if
     * writeThrough is asked for and it cannot be written.
     */
    void alloc(const fs::path &path);

    // Releases the mapping, and the file if it was ours
    void dealloc();


    //
    // Querying
    //

public:

    // The file behind this buffer, empty if there is none
    const fs::path &path() const { return url; }

    // Length of the buffer in bytes
    isize size() const { return bytes; }

    // The mapped memory, nullptr if the buffer is empty
    u8 *raw() { return addr; }
    const u8 *raw() const { return addr; }

    // Returns true if this buffer has no storage
    bool empty() const { return addr == nullptr; }


    //
    // Persisting
    //

public:

    /* Updates the underlying file.
     *
     * Does nothing when writeThrough is set, because the file is already
     * being written, and nothing when readOnly is set, because it cannot be.
     * Otherwise the whole buffer is written back.
     */
    void flush();

private:

    // Maps the file at 'url', which must already exist
    void map();
};

}
