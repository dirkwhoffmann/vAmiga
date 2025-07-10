// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "FSBlock.h"
#include "FSDescriptor.h"
#include "FSObjects.h"
#include "FSTree.h"
#include "FSStorage.h"
#include "FSDoctor.h"
#include "ADFFile.h"
#include "HDFFile.h"
#include <stack>
#include <unordered_set>

namespace vamiga {

class ADFFile;
class HDFFile;
class FloppyDrive;
class HardDrive;

/* An object of type FileSystem represents an Amiga file system (OFS or FFS).
 * It is a logical volume that can be created from an ADF or HDF. In the latter
 * case, each partition can be converted into a file system individually. The
 * class provides functions for analyzing the volume's integrity, as well as
 * for reading files and directories.
 *
 * The MutableFileSystem class extends FileSystem by adding functions for
 * modifying the contents of the file system. It allows the creation of empty
 * file systems of a specified type and provides functions for manipulating
 * files and directories, such as creating, deleting, or moving items.
 */
class FileSystem : public CoreObject, public Inspectable<FSInfo, FSStats> {

    friend struct FSBlock;
    friend class  FSDoctor;
    friend struct FSHashTable;
    friend struct FSPartition;

public:

    // Disk doctor
    FSDoctor doctor = FSDoctor(*this);

protected:

    // Static file system properties
    FSTraits traits;

    // Block storage
    FSStorage storage = FSStorage(this);

    // Location of the root block
    Block rootBlock = 0;

    // Location of the current directory
    Block current = 0;

    // Location of bitmap blocks and extended bitmap blocks
    std::vector<Block> bmBlocks;
    std::vector<Block> bmExtBlocks;


    //
    // Initializing
    //
    
public:
    
    FileSystem() { stats = {}; };
    FileSystem(const MediaFile &file, isize part = 0) : FileSystem() { init(file, part); }
    FileSystem(const ADFFile &adf) : FileSystem() { init(adf); }
    FileSystem(const HDFFile &hdf, isize part = 0) : FileSystem() { init(hdf, part); }
    FileSystem(const FloppyDrive &dfn) : FileSystem() { init(dfn); }
    FileSystem(const HardDrive &hdn, isize part = 0) : FileSystem() { init(hdn, part); }

    virtual ~FileSystem();

    void init(const FSDescriptor &layout, u8 *buf, isize len);
    void init(const MediaFile &file, isize part);
    void init(const ADFFile &adf);
    void init(const HDFFile &hdf, isize part);
    void init(const FloppyDrive &dfn);
    void init(const HardDrive &hdn, isize part);

    bool isInitialized() const noexcept;
    bool isFormatted() const noexcept;


    //
    // Methods from CoreObject
    //
    
protected:
    
    const char *objectName() const noexcept override { return "FileSystem"; }
    void _dump(Category category, std::ostream &os) const noexcept override;


    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(FSInfo &result) const noexcept override;
    void cacheStats(FSStats &result) const noexcept override;


    //
    // Querying file system properties
    //

public:

    // Returns static file system properties
    const FSTraits &getTraits() const noexcept { return traits; }

    // Returns capacity information
    isize numBlocks() const noexcept { return storage.numBlocks(); }
    isize numBytes() const noexcept { return storage.numBytes(); }
    isize blockSize() const noexcept { return storage.blockSize(); }

    // Analyzes the root block
    FSName getName() const noexcept;
    string getCreationDate() const noexcept;
    string getModificationDate() const noexcept;

    // Analyzes the boot block
    string getBootBlockName() const noexcept;
    BootBlockType bootBlockType() const noexcept;
    bool hasVirus() const noexcept { return bootBlockType() == BootBlockType::VIRUS; }


    //
    // Querying block properties
    //

public:

    // Returns the type of a certain block or a block item
    FSBlockType typeof(Block nr) const noexcept;
    bool is(Block nr, FSBlockType t) const noexcept { return typeof(nr) == t; }
    FSItemType typeof(Block nr, isize pos) const noexcept;

    // Convenience wrappers
    bool isEmpty(Block nr) const noexcept { return typeof(nr) == FSBlockType::EMPTY; }

protected:

    // Predicts the type of a block
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


    //
    // Managing the block allocation bitmap
    //

public:
    
    // Checks if a block is allocated or unallocated
    bool isUnallocated(Block nr) const noexcept;
    bool isAllocated(Block nr) const noexcept { return !isUnallocated(nr); }

    // Returns the number of allocated or unallocated blocks
    isize numUnallocated() const noexcept;
    isize numAllocated() const noexcept { return numBlocks() - numUnallocated(); }

protected:
    
    // Locates the allocation bit for a certain block
    FSBlock *locateAllocationBit(Block nr, isize *byte, isize *bit) noexcept;
    const FSBlock *locateAllocationBit(Block nr, isize *byte, isize *bit) const noexcept;

    // Translate the bitmap into to a vector with the n-bit set iff the n-th block is free
    std::vector<u32> serializeBitmap() const;


    //
    // Managing files and directories
    //

public:

    // Returns the root of the directory tree
    FSBlock &root() { return at(rootBlock); }
    const FSBlock &root() const { return at(rootBlock); }

    // Returns the working directory
    FSBlock &pwd() { return at(current); }
    const FSBlock &pwd() const { return at(current); }

    // Returns the parent directory
    FSBlock &parent(const FSBlock &block);
    FSBlock *parent(const FSBlock *block) noexcept;
    const FSBlock &parent(const FSBlock &block) const;
    const FSBlock *parent(const FSBlock *block) const noexcept;

    // Changes the working directory
    void cd(const FSName &name);
    void cd(const FSBlock &path);
    void cd(const string &path);

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
    const FSBlock &seek(const FSBlock &v, const string &name) const;

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


    //
    // Traversing linked lists
    //

public:
    
    // Follows a linked list and collects all blocks
    std::vector<const FSBlock *> collect(const FSBlock &block,
                                         std::function<const FSBlock *(const FSBlock *)> next) const;
    std::vector<Block> collect(const Block nr,
                               std::function<const FSBlock *(const FSBlock *)> next) const;

    // Collects blocks of a certain type
    std::vector<const FSBlock *> collectDataBlocks(const FSBlock &block) const;
    std::vector<const FSBlock *> collectListBlocks(const FSBlock &block) const;
    std::vector<const FSBlock *> collectHashedBlocks(const FSBlock &block, isize bucket) const;
    std::vector<const FSBlock *> collectHashedBlocks(const FSBlock &block) const;
    std::vector<Block> collectDataBlocks(Block nr) const;
    std::vector<Block> collectListBlocks(Block nr) const;
    std::vector<Block> collectHashedBlocks(Block nr, isize bucket) const;
    std::vector<Block> collectHashedBlocks(Block nr) const;


    //
    // Argument checkers
    //

public:

    void require_initialized() const;
    void require_formatted() const;
    void require_file_or_directory(const FSBlock &block) const;


    //
    // GUI helper functions
    //

public:

    // Returns a portion of the block as an ASCII dump
    string ascii(Block nr, isize offset, isize len) const noexcept;

    // Returns a block summary for creating the block usage image
    void createUsageMap(u8 *buffer, isize len) const;

    // Returns a usage summary for creating the block allocation image
    void createAllocationMap(u8 *buffer, isize len) const;

    // Returns a block summary for creating the diagnose image
    void createHealthMap(u8 *buffer, isize len) const;
    
    // Searches the block list for a block of a specific type
    isize nextBlockOfType(FSBlockType type, Block after) const;
};

}
