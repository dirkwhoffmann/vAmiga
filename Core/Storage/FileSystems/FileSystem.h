// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

/* The FileSystem class represents an Amiga file system (OFS or FFS).
 * It models a logical volume that can be created on top of, e.g., an ADF file,
 * an HDF file, or an MFM-encoded FloppyDisk. In the case of an HDF, the file
 * system may span either the entire HDF or a single partition, only.
 *
 * The FileSystem class is organized as a layered architecture to separate
 * responsibilities and to enforce downward-only dependencies.
 *
 *                  Layer view                         Class view
 *
 *            -----------------------           -----------------------
 * Layer 4:  |     POSIX layer       |  <--->  |    PosixFileSystem    |
 *            -----------------------           -----------------------
 *                      |                                 / \
 *                      |                                 \ /
 *                      V                                  |
 *            -----------------------           -----------------------
 * Layer 3:  |      Path layer       |  <--->  |                       |
 *            -----------------------          |                       |
 *                      |                      |                       |
 *                      |                      |      FileSystem       |
 *                      V                      |                       |
 *            -----------------------          |                       |
 * Layer 2:  |      Path layer       |  <--->  |                       |
 *            -----------------------           -----------------------
 *                      |                                / \
 *                      |                                \ /
 *                      V                                 |
 *            -----------------------           -----------------------
 * Layer 1:  |   Block cache layer   |  <--->  |   FSCache / FSBlock   |
 *            -----------------------           -----------------------
 *                      |                                / \
 *                      |                                \ /
 *                      V                                 |
 *            -----------------------           -----------------------
 * Layer 0:  |   "Physical" device   |  <--->  |      BlockDevice      |
 *            -----------------------           -----------------------
 *
 *
 * Notes:
 *
 *   POSIX layer:
 *
 *   The uppermost layer implements a POSIX-like file system interface. It
 *   wraps a FileSystem instance and hides all lower-level access mechanisms.
 *   This layer exposes a high-level API with POSIX-style semantics, including
 *   operations such as open, close, read, write, and file-handle management.
 *
 *   Path Layer:
 *
 *   This layer is part of the FileSystem class. It resolves symbolic and
 *   relative paths into canonical file system objects. This layer is
 *   responsible for path normalization and name resolution.
 *
 *   Node Layer:
 *
 *   Interprets storage blocks as files and directories according to OFS or FFS
 *   semantics. It provides primitives for creating and deleting files and
 *   directories, as well as for accessing and modifying file metadata.
 *
 *   Block Cache Layer:
 *
 *   Bridges the node layer and the underlying block device. It manages cached
 *   access to blocks and maintains block-level metadata to improve performance
 *   and consistency.
 *
 *   Block Device Layer:
 *
 *   Provides access to the physical or virtual storage medium and stores the
 *   actual data. Any object implementing the BlockDevice protocol can serve as
 *   a backing store, including ADFFile, HDFFile, or FloppyDisk.
 */

#pragma once

#include "FSTypes.h"
#include "FSError.h"
#include "FSBlock.h"
#include "FSContract.h"
#include "FSDescriptor.h"
#include "FSObjects.h"
#include "FSTree.h"
#include "FSCache.h"
#include "FSDoctor.h"
#include "FSAllocator.h"
#include "FSImporter.h"
#include "FSExporter.h"
#include "DeviceError.h"
#include "BlockVolume.h"
#include "utl/abilities/Loggable.h"

namespace vamiga {

class ADFFile;
class HDFFile;
class FloppyDrive;
class HardDrive;

class FileSystem : public Loggable {

    friend struct FSBlock;
    friend class  FSCache;
    friend class  FSExtension;
    friend class  FSDoctor;
    friend class  FSAllocator;
    friend struct FSHashTable;
    friend struct FSPartition;
    friend struct FSTree;

    // Static file system properties
    FSTraits traits;

public:

    // Contracts
    FSRequire require = FSRequire(*this);
    FSEnsure ensure = FSEnsure(*this);


    //
    // Layer 0: Blocks
    //

private:

    // Gateway to the "physical" block device
    FSCache cache;

    // Allocation and allocation map managenent
    FSAllocator allocator = FSAllocator(*this);


    //
    // Layer 1: Nodes
    //

    // Location of the root block
    BlockNr rootBlock = 0;

    // Location of bitmap blocks and extended bitmap blocks
    std::vector<BlockNr> bmBlocks;
    std::vector<BlockNr> bmExtBlocks;


    //
    // Layer 2: Paths
    //

    // Location of the current directory
    BlockNr current = 0;


    //
    // Layer 3: Services
    //

public:

    //Subcomponents
    FSDoctor doctor = FSDoctor(*this);
    FSImporter importer = FSImporter(*this);
    FSExporter exporter = FSExporter(*this);


    //
    // Initializing
    //

public:

    FileSystem(Volume &vol);
    virtual ~FileSystem() = default;

    FileSystem(const FileSystem &fs) = delete;
    FileSystem& operator=(const FileSystem&) = delete;

    bool isFormatted() const noexcept;


    //
    // Printing debug information
    //

public:

    void dumpInfo(std::ostream &ss = std::cout) const noexcept;
    void dumpState(std::ostream &ss = std::cout) const noexcept;
    void dumpProps(std::ostream &ss = std::cout) const noexcept;
    void dumpBlocks(std::ostream &ss = std::cout) const noexcept;


    //
    // Querying file system properties
    //

public:

    // Returns static file system properties
    const FSTraits &getTraits() const noexcept { return traits; }
    isize blocks() const noexcept { return traits.blocks; }
    isize bytes() const noexcept { return traits.bytes; }
    isize bsize() const noexcept { return traits.bsize; }

    // Returns usage information and root metadata
    FSStat stat() const noexcept;

    // Returns information about the boot block
    FSBootStat bootStat() const noexcept;

    // Returns information about file permissions
    FSAttr attr(BlockNr nr) const;
    FSAttr attr(const FSBlock &fhd) const;


    // -------------------------------------------------------------------------
    //                             Layer 1: Blocks
    // -------------------------------------------------------------------------

    //
    // Querying block properties
    //

public:

    // Returns the type of a certain block or a block item
    FSBlockType typeOf(BlockNr nr) const noexcept;
    FSItemType typeOf(BlockNr nr, isize pos) const noexcept;

    // Convenience wrappers
    bool is(BlockNr nr, FSBlockType t) const noexcept { return typeOf(nr) == t; }
    bool isEmpty(BlockNr nr) const noexcept { return is(nr, FSBlockType::EMPTY); }

    // Predicts the type of a block based on the stored data
    static FSBlockType predictType(FSDescriptor &layout, BlockNr nr, const u8 *buf) noexcept;
    FSBlockType predictType(BlockNr nr, const u8 *buf) const noexcept;


    //
    // Accessing the block storage
    //

public:

    // Returns a pointer to a block with read permissions (maybe null)
    const FSBlock *tryFetch(BlockNr nr) const noexcept { return cache.tryFetch(nr); }
    const FSBlock *tryFetch(BlockNr nr, FSBlockType type) const noexcept { return cache.tryFetch(nr, type); }
    const FSBlock *tryFetch(BlockNr nr, std::vector<FSBlockType> types) const noexcept { return cache.tryFetch(nr, types); }

    // Returns a reference to a block with read permissions (may throw)
    const FSBlock &fetch(BlockNr nr) const { return cache.fetch(nr); }
    const FSBlock &fetch(BlockNr nr, FSBlockType type) const { return cache.fetch(nr, type); }
    const FSBlock &fetch(BlockNr nr, std::vector<FSBlockType> types) const { return cache.fetch(nr, types); }

    // Returns a reference to a block with write permissions (may throw)
    FSBlock &mutate(BlockNr nr) const { return cache.fetch(nr).mutate(); }
    FSBlock &mutate(BlockNr nr, FSBlockType type) const { return cache.fetch(nr, type).mutate(); }
    FSBlock &mutate(BlockNr nr, std::vector<FSBlockType> types) const { return cache.fetch(nr, types).mutate(); }

    // Returns a pointer to a block with write permissions (maybe null)
    FSBlock *tryModify(BlockNr nr) noexcept { return cache.tryModify(nr); }
    FSBlock *tryModify(BlockNr nr, FSBlockType type) noexcept { return cache.tryModify(nr, type); }
    FSBlock *tryModify(BlockNr nr, std::vector<FSBlockType> types) noexcept { return cache.tryModify(nr, types); }

    // Returns a reference to a block with write permissions (may throw)
    [[deprecated]] FSBlock &modify(BlockNr nr) { return cache.modify(nr); }
    [[deprecated]] FSBlock &modify(BlockNr nr, FSBlockType type) { return cache.modify(nr, type); }
    [[deprecated]] FSBlock &modify(BlockNr nr, std::vector<FSBlockType> types) { return cache.modify(nr, types); }

    // Writes back dirty cache blocks to the block device
    void flush();

    // Operator overload
    // FSBlock &operator[](size_t nr);
    const FSBlock &operator[](size_t nr);


    // -------------------------------------------------------------------------
    //                             Layer 2: Nodes
    // -------------------------------------------------------------------------

    //
    // Formatting
    //

public:

    // Formats the volume
    void format(string name = "");
    void format(FSFormat dos, string name = "");

    // Assigns the volume name
    void setName(FSName name);
    void setName(string name) { setName(FSName(name)); }

    // Installs a boot block
    void makeBootable(BootBlockId id);

    // Removes a boot block virus from the current partition (if any)
    void killVirus();


    //
    // Managing directories
    //

    // Creates a new directory
    BlockNr mkdir(BlockNr at, const FSName &name);

    // Removes an empty directory
    void rmdir(BlockNr at);

    // Looks up a directory item
    optional<BlockNr> searchdir(BlockNr at, const FSName &name);

    // Creates a directory entry
    void link(BlockNr at, BlockNr fhb);

    // Removes a directory entry
    void unlink(BlockNr fhb);

private:

    // Adds a hash-table entry for a given item
    void addToHashTable(const FSBlock &item);
    void addToHashTable(BlockNr parent, BlockNr ref);

    // Removes the hash-table entry for a given item
    void deleteFromHashTable(const FSBlock &item);
    void deleteFromHashTable(BlockNr item);
    void deleteFromHashTable(BlockNr parent, BlockNr ref);


    //
    // Managing files
    //

public:

    // Creates a new file
    BlockNr createFile(BlockNr at, const FSName &name);
    BlockNr createFile(BlockNr at, const FSName &name, const u8 *buf, isize size);
    BlockNr createFile(BlockNr at, const FSName &name, const Buffer<u8> &buf);
    BlockNr createFile(BlockNr at, const FSName &name, const string &str);

    // Delete a file
    void rm(BlockNr at);

    // Renames a file or directory
    void rename(BlockNr item, const FSName &name);

    // Moves a file or directory to another location
    void move(BlockNr item, BlockNr dest);
    void move(BlockNr item, BlockNr dest, const FSName &name);

    // Copies a file
    void copy(BlockNr item, BlockNr dest);
    void copy(BlockNr item, BlockNr dest, const FSName &name);

    // Shrinks or expands an existing file (pad with 0)
    void resize(BlockNr at, isize size);

    // Replaces the cotents of an existing file
    void replace(BlockNr at, const Buffer<u8> &data);

private:

    // Main replace function
    BlockNr replace(BlockNr fhb,
                    const u8 *buf, isize size,
                    std::vector<BlockNr> listBlocks = {},
                    std::vector<BlockNr> dataBlocks = {});


    //
    // Creating and destroying blocks
    //

public:

    // Frees the blocks of a deleted directory or file
    void reclaim(BlockNr fhb);

private:

    // Creates a new block of a certain kind
    FSBlock &newUserDirBlock(const FSName &name);
    FSBlock &newFileHeaderBlock(const FSName &name);

    // Adds a new block of a certain kind
    void addFileListBlock(BlockNr at, BlockNr head, BlockNr prev);
    void addDataBlock(BlockNr at, isize id, BlockNr head, BlockNr prev);

    // Adds bytes to a data block
    isize addData(BlockNr nr, const u8 *buf, isize size);


    //
    // Traversing linked lists
    //

private:

    // Follows a linked list and collects all blocks
    using BlockIterator = std::function<const FSBlock *(const FSBlock *)>;
    std::vector<const FSBlock *> collect(const FSBlock &block, BlockIterator succ) const;
    std::vector<BlockNr> collect(const BlockNr nr, BlockIterator succ) const;

    // Collects blocks of a certain type
    std::vector<const FSBlock *> collectDataBlocks(const FSBlock &block) const;
    std::vector<const FSBlock *> collectListBlocks(const FSBlock &block) const;
    std::vector<const FSBlock *> collectHashedBlocks(const FSBlock &block, isize bucket) const;
    std::vector<const FSBlock *> collectHashedBlocks(const FSBlock &block) const;
    std::vector<BlockNr> collectDataBlocks(BlockNr nr) const;
    std::vector<BlockNr> collectListBlocks(BlockNr nr) const;
    std::vector<BlockNr> collectHashedBlocks(BlockNr nr, isize bucket) const;
    std::vector<BlockNr> collectHashedBlocks(BlockNr nr) const;


    // -------------------------------------------------------------------------
    //                           Layer 3: Paths
    // -------------------------------------------------------------------------

    //
    // Managing the working directory
    //

public:

    // Returns the working directory
    BlockNr pwd() const { return current; } // RENAME LATER TO pwd()
    [[deprecated]] FSBlock &deprecatedPwd() { return modify(current); } // TODO: DEPRECATE ASAP
    [[deprecated]] const FSBlock &deprecatedPwd() const { return fetch(current); } // TODO: DEPRECATE ASAP

    // Changes the working directory
    void cd(BlockNr nr);
    void cd(const FSName &name);
    void cd(const string &path);


    //
    // Seeking files and directories
    //

public:

    // Returns the root of the directory tree
    BlockNr root() { return rootBlock; } // RENAME LATER TO root()
    [[deprecated]] FSBlock &deprecatedRoot() { return modify(rootBlock); } // DEPRECATED
    [[deprecated]] const FSBlock &deprecatedRoot() const { return fetch(rootBlock); }

    // Returns the parent directory of an item
    [[deprecated]] FSBlock &parent(const FSBlock &block);
    [[deprecated]] FSBlock *parent(const FSBlock *block) noexcept;
    [[deprecated]] const FSBlock &parent(const FSBlock &block) const;
    [[deprecated]] const FSBlock *parent(const FSBlock *block) const noexcept;

    // Checks if a an item exists in the directory tree
    bool exists(BlockNr top, const fs::path &path) const;
    bool exists(const fs::path &path) const { return exists(pwd(), path); }

    // Seeks an item in the directory tree (returns nullptr if not found)
    [[deprecated]] FSBlock *seekPtr(const FSBlock *top, const FSName &name) noexcept;
    [[deprecated]] FSBlock *seekPtr(const FSBlock *top, const fs::path &name) noexcept;
    [[deprecated]] FSBlock *seekPtr(const FSBlock *top, const string &name) noexcept;
    [[deprecated]] const FSBlock *seekPtr(const FSBlock *top, const FSName &name) const noexcept;
    [[deprecated]] const FSBlock *seekPtr(const FSBlock *top, const fs::path &name) const noexcept;
    [[deprecated]] const FSBlock *seekPtr(const FSBlock *top, const string &name) const noexcept;

    optional<BlockNr> trySeek(BlockNr top, const FSName &name) const;
    optional<BlockNr> trySeek(BlockNr top, const fs::path &name) const;
    optional<BlockNr> trySeek(BlockNr top, const string &name) const;

    BlockNr seek(BlockNr top, const FSName &name) const;
    BlockNr seek(BlockNr top, const fs::path &name) const;
    BlockNr seek(BlockNr top, const string &name) const;


    // Seeks an item in the directory tree
    [[deprecated]] FSBlock &seek(const FSBlock &top, const FSName &name);
    [[deprecated]] FSBlock &seek(const FSBlock &top, const fs::path &name);
    [[deprecated]] FSBlock &seek(const FSBlock &top, const string &name);
    [[deprecated]] const FSBlock &seek(const FSBlock &top, const FSName &name) const;
    [[deprecated]] const FSBlock &seek(const FSBlock &top, const fs::path &name) const;
    [[deprecated]] const FSBlock &seek(const FSBlock &top, const string &name) const;

    // Seeks all items satisfying a predicate
    std::vector<const FSBlock *> find(const FSOpt &opt) const;
    std::vector<const FSBlock *> find(const FSBlock *root, const FSOpt &opt) const;
    std::vector<const FSBlock *> find(const FSBlock &root, const FSOpt &opt) const;
    std::vector<BlockNr> find(BlockNr root, const FSOpt &opt) const;

    // Seeks all items with a pattern-matching name
    std::vector<const FSBlock *> find(const FSPattern &pattern) const;
    std::vector<const FSBlock *> find(const FSBlock *top, const FSPattern &pattern) const;
    std::vector<const FSBlock *> find(const FSBlock &top, const FSPattern &pattern) const;
    std::vector<BlockNr> find(BlockNr root, const FSPattern &pattern) const;

    // Collects all items with a pattern-matching path
    std::vector<const FSBlock *> match(const FSPattern &pattern) const;
    std::vector<const FSBlock *> match(const FSBlock *top, const FSPattern &pattern) const;
    std::vector<const FSBlock *> match(const FSBlock &top, const FSPattern &pattern) const;
    std::vector<BlockNr> match(BlockNr root, const FSPattern &pattern) const;

private:

    std::vector<const FSBlock *> find(const FSBlock *top, const FSOpt &opt,
                                      std::unordered_set<BlockNr> &visited) const;

    std::vector<const FSBlock *> match(const FSBlock *top,
                                       std::vector<FSPattern> pattern) const;
};


//
// Argument checkers
//

namespace require {

void formatted(const FileSystem &fs);
void formatted(unique_ptr<FileSystem> &fs);

void file(const FSBlock &node);
void directory(const FSBlock &block);
void fileOrDirectory(const FSBlock &block);
void notRoot(const FSBlock &block);
void emptyDirectory(const FSBlock &block);
void notExist(const FSBlock &dir, const FSName &name);
}

}
