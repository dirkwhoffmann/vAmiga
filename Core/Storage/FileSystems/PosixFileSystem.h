// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FileSystem.h"
#include <fcntl.h>

namespace vamiga {

struct Handle {

    // Unique indentifier
    isize id = 0;

    // File header block
    BlockNr headerBlock = 0;

    // I/O offset
    isize offset = 0;

    // Open mode (POSIX-style flags)
    u32 flags = 0;
};

using HandleRef = isize;

struct NodeMeta {

    // Number of directory entries
    isize linkCount = 1;

    // All open handles referencing this node
    std::unordered_set<HandleRef> openHandles;

    // File cache
    Buffer<u8> cache;

    // Returns the number of open handles
    isize openCount() { return openHandles.size(); };
};

class PosixFileSystem {

    // The wrapped file system
    FileSystem &fs;

    // Contracts
    FSRequire require = FSRequire(fs);
    FSEnsure ensure = FSEnsure(fs);

    // Metadata for nodes indexed by block number
    std::unordered_map<BlockNr, NodeMeta> meta;

    // Active file handles
    std::unordered_map<HandleRef, Handle> handles;

    // Handle ID generator
    isize nextHandle = 3;

public:

    explicit PosixFileSystem(FileSystem &fs);

    //
    // Querying statistics and properties
    //

public:

    // Queries information about the file system
    FSStat stat() const noexcept;

    // Queries information about a specific file
    FSAttr attr(const fs::path &path) const;


    //
    // Managing metadata
    //

private:

    // Returns a pointer to the meta struct (may be nullptr)
    NodeMeta *getMeta(BlockNr nr);
    NodeMeta *getMeta(const FSBlock &block) { return getMeta(block.nr); }

    // Returns a reference to the meta struct (on-the-fly creation)
    NodeMeta &ensureMeta(BlockNr nr);
    NodeMeta &ensureMeta(const FSBlock &block) { return ensureMeta(block.nr); }
    NodeMeta &ensureMeta(HandleRef ref);


    //
    // Working with directories
    //

public:

    // Creates a directory
    void mkdir(const fs::path &path);

    // Removes a directory
    void rmdir(const fs::path &path);

    // Returns the contents of a directory
    std::vector<FSName> readDir(const fs::path &path);


    //
    // Working with files
    //

public:

    // Opens or closes a file
    HandleRef open(const fs::path &path, u32 flags);
    void close(HandleRef handle);

    // Creates a new file
    void create(const fs::path &path);

    // Removes a file from its directory
    void unlink(const fs::path &path);

    // Moves the file to a different location
    void move(const fs::path &oldPath, const fs::path &newPath);

    // Changes the size of a file
    void resize(const fs::path &path, isize size);

    // Moves the read/write pointer
    isize lseek(HandleRef ref, isize offset, u16 whence = 0);

    // Reads data from a file
    isize read(HandleRef ref, std::span<u8> buffer);

    // Writes data to a file
    isize write(HandleRef ref, std::span<const u8> buffer);

    // Changes file permissions
    void chmod(const fs::path &path, mode_t mode);


private:

    void tryReclaim(BlockNr block);

    Handle &getHandle(HandleRef ref);

    BlockNr ensureFile(const fs::path &path);
    BlockNr ensureFileOrDirectory(const fs::path &path);
    BlockNr ensureDirectory(const fs::path &path);
};

}
