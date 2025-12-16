// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

/* The FileSystem class represents an Amiga file system (OFS or FFS).
 * It models a logical volume that can be created from either an ADF or an HDF.
 * In the case of an HDF, each partition can be converted into an independent
 * file system instance.
 *
 * The FileSystem class is organized as a layered architecture to separate
 * responsibilities and to enforce downward-only dependencies.
 *
 *  ---------------------
 * |     POSIX Layer     |    Layer 3:
 *  ---------------------
 *            |               Wraps a Layer-3 file system with all lower-level
 *            |               access functions hidden. It exposes a POSIX-like
 *            |               high-level API that provides operations such as
 *            |               open, close, read, write, and file handles.
 *            V
 *  -----------------------
 * | Path Resolution Layer |  Layer 2:
 *  -----------------------
 *            |               Resolves symbolic and relative paths into file
 *            |               system objects and canonicalizes paths. It depends
 *            |               only on the read/write layer.
 *            |
 *            V
 *  -----------------------
 * |       Node Layer      |  Layer 1:
 *  -----------------------
 *            |               Interprets storage blocks as files and directories
 *            |               according to OFS or FFS semantics. It allows the
 *            |               upper layers to create files, directories, and to
 *            |               modify metadata.
 *            V
 *  -----------------------
 * |  Block Storage Layer  |  Layer 0:
 *  -----------------------
 *                            Storage the actual block data.
 *
 */

#include "FSTypes.h"
#include "FSError.h"
#include "FSBlock.h"
#include "FSDescriptor.h"
#include "FSObjects.h"
#include "FSTree.h"
#include "FSStorage.h"
#include "FSDoctor.h"
#include "FSAllocator.h"
#include "FSImporter.h"
#include "FSExporter.h"
#include "DeviceError.h"
#include "Device.h"
#include "utl/abilities/Loggable.h"

namespace vamiga {

class ADFFile;
class HDFFile;
class FloppyDrive;
class HardDrive;

class FileSystem : public Loggable {

    friend struct FSBlock;
    friend class  FSStorage;
    friend class  FSExtension;
    friend class  FSDoctor;
    friend class  FSAllocator;
    friend struct FSHashTable;
    friend struct FSPartition;
    friend struct FSTree;

    // Static file system properties
    FSTraits traits;

    // The underlying block device
    BlockView &dev;


    //
    // Layer 0
    //

    // Block storage
    FSStorage storage = FSStorage(*this);

    // Allocation and allocation map managenent
    FSAllocator allocator = FSAllocator(*this);


    //
    // Layer 1
    //

    // Location of the root block
    Block rootBlock = 0;

    // Location of bitmap blocks and extended bitmap blocks
    std::vector<Block> bmBlocks;
    std::vector<Block> bmExtBlocks;


    //
    // Layer 2
    //

    // Location of the current directory
    Block current = 0;


public:

    //Subcomponents
    FSDoctor doctor = FSDoctor(*this);
    FSImporter importer = FSImporter(*this);
    FSExporter exporter = FSExporter(*this);


    //
    // Initializing
    //

public:

    FileSystem(BlockView &device) : dev(device) { };
    FileSystem(BlockView &device, isize capacity, isize bsize = 512) : FileSystem(device) { init(capacity, bsize); }
    FileSystem(BlockView &device, const FSDescriptor &layout, u8 *buf, isize len) : FileSystem(device) { init(layout, buf, len); }
    FileSystem(BlockView &device, const FSDescriptor &layout, const fs::path &path = {})  : FileSystem(device) { init(layout, path); }
    FileSystem(BlockView &device, const FileSystem &fs) = delete;
    virtual ~FileSystem() = default;

    void init(isize capacity, isize bsize = 512);
    void init(const FSDescriptor &layout, u8 *buf, isize len);
    void init(const FSDescriptor &layout, const fs::path &path = {});

    bool isInitialized() const noexcept;
    bool isFormatted() const noexcept;

    FileSystem& operator=(const FileSystem&) = delete;


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
    FSAttr attr(Block nr) const;
    FSAttr attr(const FSBlock &fhd) const;


    // -------------------------------------------------------------------------
    //                             Layer 0: Blocks
    // -------------------------------------------------------------------------

    //
    // Querying block properties
    //

public:

    // Returns the type of a certain block or a block item
    FSBlockType typeOf(Block nr) const noexcept;
    FSItemType typeOf(Block nr, isize pos) const noexcept;

    // Convenience wrappers
    bool is(Block nr, FSBlockType t) const noexcept { return typeOf(nr) == t; }
    bool isEmpty(Block nr) const noexcept { return is(nr, FSBlockType::EMPTY); }

    // Predicts the type of a block based on the stored data
    FSBlockType predictType(Block nr, const u8 *buf) const noexcept;


    //
    // Accessing the block storage
    //

public:

    // Returns a block pointer or null if the block does not exist
    FSBlock *read(Block nr) noexcept;
    FSBlock *read(Block nr, FSBlockType type) noexcept;
    FSBlock *read(Block nr, std::vector<FSBlockType> types) noexcept;
    const FSBlock *read(Block nr) const noexcept;
    const FSBlock *read(Block nr, FSBlockType type) const noexcept;
    const FSBlock *read(Block nr, std::vector<FSBlockType> types) const noexcept;

    // Returns a reference to a stored block (throws on error)
    FSBlock &at(Block nr);
    FSBlock &at(Block nr, FSBlockType type);
    FSBlock &at(Block nr, std::vector<FSBlockType> types);
    const FSBlock &at(Block nr) const;
    const FSBlock &at(Block nr, FSBlockType type) const;
    const FSBlock &at(Block nr, std::vector<FSBlockType> types) const;

    // Operator overload
    FSBlock &operator[](size_t nr);
    const FSBlock &operator[](size_t nr) const;


    // -------------------------------------------------------------------------
    //                             Layer 1: Nodes
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
    FSBlock &mkdir(FSBlock &at, const FSName &name);

    // Removes an empty directory
    void rmdir(FSBlock &at);

    // Looks up a directory item
    FSBlock *searchdir(const FSBlock &at, const FSName &name);

    // Creates a directory entry
    void link(FSBlock &at, FSBlock &fhb);

    // Removes a directory entry
    void unlink(const FSBlock &fhb);

private:

    // Adds a hash-table entry for a given item
    void addToHashTable(const FSBlock &item);
    void addToHashTable(Block parent, Block ref);

    // Removes the hash-table entry for a given item
    void deleteFromHashTable(const FSBlock &item);
    void deleteFromHashTable(Block parent, Block ref);


    //
    // Managing files
    //

public:

    // Creates a new file
    FSBlock &createFile(FSBlock &at, const FSName &name);
    FSBlock &createFile(FSBlock &at, const FSName &name, const u8 *buf, isize size);
    FSBlock &createFile(FSBlock &at, const FSName &name, const Buffer<u8> &buf);
    FSBlock &createFile(FSBlock &at, const FSName &name, const string &str);

    // Delete a file
    void rm(const FSBlock &at);

    // Renames a file or directory
    void rename(FSBlock &item, const FSName &name);

    // Moves a file or directory to another location
    void move(FSBlock &item, FSBlock &dest);
    void move(FSBlock &item, FSBlock &dest, const FSName &name);

    // Copies a file
    void copy(const FSBlock &item, FSBlock &dest);
    void copy(const FSBlock &item, FSBlock &dest, const FSName &name);

    // Shrinks or expands an existing file (pad with 0)
    void resize(FSBlock &at, isize size);

    // Replaces the cotents of an existing file
    void replace(FSBlock &at, const Buffer<u8> &data);

private:

    // Main replace function
    FSBlock &replace(FSBlock &fhb,
                     const u8 *buf, isize size,
                     std::vector<Block> listBlocks = {},
                     std::vector<Block> dataBlocks = {});


    //
    // Creating and destroying blocks
    //

public:

    // Frees the blocks of a deleted directory or file
    void reclaim(const FSBlock &fhb);

private:

    // Creates a new block of a certain kind
    FSBlock &newUserDirBlock(const FSName &name);
    FSBlock &newFileHeaderBlock(const FSName &name);

    // Adds a new block of a certain kind
    void addFileListBlock(Block at, Block head, Block prev);
    void addDataBlock(Block at, isize id, Block head, Block prev);

    // Adds bytes to a data block
    isize addData(Block nr, const u8 *buf, isize size);
    isize addData(FSBlock &block, const u8 *buf, isize size);


    //
    // Traversing linked lists
    //

private:

    // Follows a linked list and collects all blocks
    using BlockIterator = std::function<const FSBlock *(const FSBlock *)>;
    std::vector<const FSBlock *> collect(const FSBlock &block, BlockIterator succ) const;
    std::vector<Block> collect(const Block nr, BlockIterator succ) const;

    // Collects blocks of a certain type
    std::vector<const FSBlock *> collectDataBlocks(const FSBlock &block) const;
    std::vector<const FSBlock *> collectListBlocks(const FSBlock &block) const;
    std::vector<const FSBlock *> collectHashedBlocks(const FSBlock &block, isize bucket) const;
    std::vector<const FSBlock *> collectHashedBlocks(const FSBlock &block) const;
    std::vector<Block> collectDataBlocks(Block nr) const;
    std::vector<Block> collectListBlocks(Block nr) const;
    std::vector<Block> collectHashedBlocks(Block nr, isize bucket) const;
    std::vector<Block> collectHashedBlocks(Block nr) const;


    // -------------------------------------------------------------------------
    //                           Layer 2: Paths
    // -------------------------------------------------------------------------

    //
    // Managing the working directory
    //

public:
    
    // Returns the working directory
    FSBlock &pwd() { return at(current); }
    const FSBlock &pwd() const { return at(current); }

    // Changes the working directory
    void cd(const FSName &name);
    void cd(const FSBlock &path);
    void cd(const string &path);


    //
    // Seeking files and directories
    //

public:

    // Returns the root of the directory tree
    FSBlock &root() { return at(rootBlock); }
    const FSBlock &root() const { return at(rootBlock); }

    // Returns the parent directory of an item
    FSBlock &parent(const FSBlock &block);
    FSBlock *parent(const FSBlock *block) noexcept;
    const FSBlock &parent(const FSBlock &block) const;
    const FSBlock *parent(const FSBlock *block) const noexcept;

    // Checks if a an item exists in the directory tree
    bool exists(const FSBlock &top, const fs::path &path) const;
    bool exists(const fs::path &path) const { return exists(pwd(), path); }

    // Seeks an item in the directory tree (returns nullptr if not found)
    FSBlock *seekPtr(const FSBlock *top, const FSName &name) noexcept;
    FSBlock *seekPtr(const FSBlock *top, const fs::path &name) noexcept;
    FSBlock *seekPtr(const FSBlock *top, const string &name) noexcept;
    const FSBlock *seekPtr(const FSBlock *top, const FSName &name) const noexcept;
    const FSBlock *seekPtr(const FSBlock *top, const fs::path &name) const noexcept;
    const FSBlock *seekPtr(const FSBlock *top, const string &name) const noexcept;

    // Seeks an item in the directory tree (returns nullptr if not found)
    FSBlock &seek(const FSBlock &top, const FSName &name);
    FSBlock &seek(const FSBlock &top, const fs::path &name);
    FSBlock &seek(const FSBlock &top, const string &name);
    const FSBlock &seek(const FSBlock &top, const FSName &name) const;
    const FSBlock &seek(const FSBlock &top, const fs::path &name) const;
    const FSBlock &seek(const FSBlock &top, const string &name) const;

    // Seeks all items satisfying a predicate
    std::vector<const FSBlock *> find(const FSOpt &opt) const;
    std::vector<const FSBlock *> find(const FSBlock *root, const FSOpt &opt) const;
    std::vector<const FSBlock *> find(const FSBlock &root, const FSOpt &opt) const;
    std::vector<Block> find(Block root, const FSOpt &opt) const;

    // Seeks all items with a pattern-matching name
    std::vector<const FSBlock *> find(const FSPattern &pattern) const;
    std::vector<const FSBlock *> find(const FSBlock *top, const FSPattern &pattern) const;
    std::vector<const FSBlock *> find(const FSBlock &top, const FSPattern &pattern) const;
    std::vector<Block> find(Block root, const FSPattern &pattern) const;

    // Collects all items with a pattern-matching path
    std::vector<const FSBlock *> match(const FSPattern &pattern) const;
    std::vector<const FSBlock *> match(const FSBlock *top, const FSPattern &pattern) const;
    std::vector<const FSBlock *> match(const FSBlock &top, const FSPattern &pattern) const;
    std::vector<Block> match(Block root, const FSPattern &pattern) const;

private:

    std::vector<const FSBlock *> find(const FSBlock *top, const FSOpt &opt,
                                      std::unordered_set<Block> &visited) const;

    std::vector<const FSBlock *> match(const FSBlock *top,
                                       std::vector<FSPattern> pattern) const;
};


//
// Argument checkers
//

namespace require {

    void initialized(const FileSystem &fs);
    void initialized(unique_ptr<FileSystem> &fs);
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
