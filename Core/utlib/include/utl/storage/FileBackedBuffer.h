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
 * Two modes decide what a write means:
 *
 *   readOnly     this buffer will never modify the file. A veto: it holds
 *                whatever writeThrough says.
 *   writeThrough writes reach the file as they happen, rather than being
 *                held in memory until flush() puts them there.
 *
 *   readOnly  writeThrough   a write to raw() ...        flush() ...
 *   ----------------------------------------------------------------------
 *   false     true           goes to the file at once    does nothing
 *   false     false          stays private to us         writes the file
 *   true      either         stays private to us         does nothing
 *
 * Both can be changed after the fact, but only through setReadOnly() and
 * setWriteThrough(), because a change may have to rebuild the mapping:
 * whether writes go to the file is fixed when a mapping is made, not when it
 * is used. The two setters take care of that, and of not losing anything on
 * the way -- switching a buffer with private modifications over to
 * write-through puts them in the file first, since that is where the new
 * mapping will look for them.
 *
 * *** raw() may change when a mode changes. ***
 *
 * Rebuilding a mapping generally moves it, so a pointer taken before
 * setReadOnly() or setWriteThrough() must not be used after. This happens
 * exactly when the mapping kind changes, that is when (writeThrough &&
 * !readOnly) flips; every other mode change leaves the mapping, and the
 * pointer, alone.
 *
 * Where the file comes from:
 *
 * - alloc(path) uses an existing file and leaves it behind when the buffer
 *   goes away. If that file cannot be opened for writing, the buffer becomes
 *   readOnly whether or not it was asked to be -- but asking for
 *   writeThrough on such a file throws, because the caller asked for writes
 *   to reach the file and they cannot.
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

    /* The two modes.
     *
     * Private, and reachable only through the setters, because assigning to
     * them could not rebuild the mapping -- it would leave a buffer whose
     * stated mode and actual behaviour disagree.
     */
    bool wt = false;
    bool ro = false;

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
    FileBackedBuffer(isize size, bool writeThrough = false, bool readOnly = false);

    // Creates a buffer backed by an existing file
    FileBackedBuffer(const fs::path &path, bool writeThrough = false,
                     bool readOnly = false);

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
    void alloc(isize size, bool writeThrough = false, bool readOnly = false);

    /* Backs the buffer with an existing file, in its entirety.
     *
     * The file outlives the buffer. Throws if it does not exist, or if
     * writeThrough is asked for and it cannot be written.
     */
    void alloc(const fs::path &path, bool writeThrough = false, bool readOnly = false);

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

    // Returns true if writes reach the file as they happen
    bool writeThrough() const { return wt; }

    // Returns true if this buffer will never modify the file
    bool readOnly() const { return ro; }


    //
    // Changing the modes
    //

public:

    /* Switches a mode, rebuilding the mapping if that is what it takes.
     *
     * Nothing happens if the value is already set, or if the buffer holds no
     * storage -- in that case the value is simply remembered for the next
     * alloc(). Otherwise see the note on raw() above: the pointer may move.
     *
     * Leaving the buffer's contents behind is not a thing either setter does.
     * A switch to write-through writes the private modifications out before
     * remapping; a switch away from it needs nothing, because the file is
     * already current.
     */
    void setWriteThrough(bool value);
    void setReadOnly(bool value);


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

    // True when the mapping is shared, that is when writes reach the file
    bool shared() const { return wt && !ro; }

    // Maps the file at 'url', which must already exist
    void map();

    // Rebuilds the mapping after a mode change
    void remap(bool wasShared);

    // Writes the buffer back to the file, whatever the modes say
    void writeBack();
};

}
