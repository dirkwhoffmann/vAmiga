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
#include "FSDescriptors.h"
#include "FSObjects.h"
#include "FSTree.h"
#include "BlockStorage.h"
#include "DiskDoctor.h"
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
 * case, each partition can be converted to a file system individually. The
 * class provides functions for analyzing the integrity of the volume as well
 * as functions for reading files and directories.
 *
 * The MutableFileSystem class extends the FileSystem class with functions for
 * modifiying the contents of the file system. It provides functions for
 * creating empty file systems of a certain type as well as functions for
 * manipulation files and directories, such as creating, deleting, or moving
 * items.
 */
class FileSystem : public CoreObject, public Inspectable<FSInfo, FSStats> {

    friend struct FSBlock;
    friend struct FSHashTable;
    friend struct FSPartition;
    friend class DiskDoctor;

public:

    // File system checker and rectifier
    DiskDoctor doctor = DiskDoctor(*this);

protected:

    // Static file system properties
    FSTraits traits;

    // Block storage
    BlockStorage storage = BlockStorage(this);

    // Location of the root block
    Block rootBlock = 0;

    // Location of the current directory
    Block current = 0;

    // Location of the bitmap blocks and extended bitmap blocks
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
    FileSystem(FloppyDrive &dfn) : FileSystem() { init(dfn); }
    FileSystem(const HardDrive &hdn, isize part = 0) : FileSystem() { init(hdn, part); }

    virtual ~FileSystem();

    void init(FileSystemDescriptor layout, u8 *buf, isize len);
    void init(const MediaFile &file, isize part);
    void init(const ADFFile &adf);
    void init(const HDFFile &hdf, isize part);
    void init(FloppyDrive &dfn);
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

    const FSTraits &getTraits() const noexcept { return traits; }

    // Returns capacity information
    isize numBlocks() const noexcept { return storage.numBlocks(); }
    isize numBytes() const noexcept { return storage.numBytes(); }
    isize blockSize() const noexcept { return storage.blockSize(); }

    // Reports usage information
    isize freeBlocks() const noexcept;
    isize usedBlocks() const noexcept;
    isize freeBytes() const noexcept { return freeBlocks() * traits.bsize; }
    isize usedBytes() const noexcept { return usedBlocks() * traits.bsize; }
    double fillLevel() const noexcept { return double(100) * usedBlocks() / numBlocks(); }

    // Reads information from the root block
    FSName getName() const noexcept;
    string getCreationDate() const noexcept;
    string getModificationDate() const noexcept;

    // Analyzes the boot block
    string getBootBlockName() const noexcept;
    BootBlockType bootBlockType() const noexcept;
    bool hasVirus() const noexcept { return bootBlockType() == BootBlockType::VIRUS; }

    
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

    // Returns the type of a certain block
    FSBlockType blockType(Block nr) const noexcept;
    bool isEmpty(Block nr) const noexcept { return blockType(nr) == FSBlockType::EMPTY_BLOCK; }

    // Returns the usage type of a certain byte in a certain block
    FSItemType itemType(Block nr, isize pos) const noexcept;

    // Queries a pointer to a block of a certain type (may return nullptr)
    FSBlock *hashableBlockPtr(Block nr) const noexcept;

    // Reads a single byte from a block
    u8 readByte(Block nr, isize offset) const noexcept;

    // Predicts the type of a block by analyzing its number and data
    FSBlockType predictBlockType(Block nr, const u8 *buf) const noexcept;


    //
    // Querying the block allocation bitmap
    //

public:
    
    // Checks if a block is free or allocated
    bool isFree(Block nr) const noexcept;
    bool isAllocated(Block nr) const noexcept { return !isFree(nr); }

protected:
    
    // Locates the allocation bit for a certain block
    FSBlock *locateAllocationBit(Block nr, isize *byte, isize *bit) noexcept;
    const FSBlock *locateAllocationBit(Block nr, isize *byte, isize *bit) const noexcept;

    
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
    FSBlock &parent(const FSBlock &node);
    FSBlock *parent(const FSBlock *node) noexcept;
    const FSBlock &parent(const FSBlock &node) const;
    const FSBlock *parent(const FSBlock *node) const noexcept;

    // Changes the working directory
    void cd(const FSName &name);
    void cd(const FSBlock &path);
    void cd(const string &path);

    // Checks if a an item exists in the directory tree
    bool exists(const FSBlock &top, const fs::path &path) const;
    bool exists(const fs::path &path) const { return exists(pwd(), path); }

    // Seeks an item in the directory tree (returns nullptr if not found)
    FSBlock *seekPtr(const FSBlock *root, const FSName &name) noexcept;
    FSBlock *seekPtr(const FSBlock *root, const fs::path &name) noexcept;
    FSBlock *seekPtr(const FSBlock *root, const string &name) noexcept;
    const FSBlock *seekPtr(const FSBlock *root, const FSName &name) const noexcept;
    const FSBlock *seekPtr(const FSBlock *root, const fs::path &name) const noexcept;
    const FSBlock *seekPtr(const FSBlock *root, const string &name) const noexcept;

    // Seeks an item in the directory tree (returns nullptr if not found)
    FSBlock &seek(const FSBlock &root, const FSName &name);
    FSBlock &seek(const FSBlock &root, const fs::path &name);
    FSBlock &seek(const FSBlock &root, const string &name);
    const FSBlock &seek(const FSBlock &root, const FSName &name) const;
    const FSBlock &seek(const FSBlock &root, const fs::path &name) const;
    const FSBlock &seek(const FSBlock &root, const string &name) const;

    // Seeks all items satisfying a predicate
    std::vector<const FSBlock *> find(const FSPattern &pattern) const;
    std::vector<const FSBlock *> find(const FSBlock *root, const FSOpt &opt) const;
    std::vector<const FSBlock *> find(const FSBlock &root, const FSOpt &opt) const;
    std::vector<Block> find(const Block root, const FSOpt &opt) const;

    // Seeks all items with a pattern-matching name
    std::vector<const FSBlock *> find(const FSBlock *root, const FSPattern &pattern) const;
    std::vector<const FSBlock *> find(const FSBlock &root, const FSPattern &pattern) const;
    std::vector<Block> find(const Block root, const FSPattern &pattern) const;

    // Collects all items with a pattern-matching path
    std::vector<const FSBlock *> match(const FSBlock *root, const FSPattern &pattern) const;
    std::vector<const FSBlock *> match(const FSBlock &root, const FSPattern &pattern) const;
    std::vector<const FSBlock *> match(const FSBlock *root, std::vector<FSPattern> pattern) const;

    // Lists the contents of a directory ('dir' command, 'list' command)
    void list(std::ostream &os, const FSBlock &path, const FSOpt &opt = {}) const;
    void list(std::ostream &os, const FSOpt &opt = {}) const { return list(os, pwd(), opt); }
    void listDirectory(std::ostream &os, const FSBlock &path, const FSOpt &opt = {}) const;
    void listItems(std::ostream &os, std::vector<const FSBlock *> items, const FSOpt &opt = {}) const;

private:

    std::vector<const FSBlock *> find(const FSBlock *root, const FSOpt &opt,
                                      std::unordered_set<Block> &visited) const;

    //
    // Traversing linked lists
    //

public:
    
    // Follows a linked list and collects all blocks
    std::vector<FSBlock *> collect(const FSBlock &node, std::function<FSBlock *(FSBlock *)> next) const;
    std::vector<Block> collect(const Block nr, std::function<FSBlock *(FSBlock *)> next) const;

    // Collects blocks of a certain type
    std::vector<FSBlock *> collectDataBlocks(const FSBlock &node) const;
    std::vector<FSBlock *> collectListBlocks(const FSBlock &node) const;
    std::vector<FSBlock *> collectHashedBlocks(const FSBlock &node, isize bucket) const;
    std::vector<FSBlock *> collectHashedBlocks(const FSBlock &node) const;
    std::vector<Block> collectDataBlocks(Block ref) const;
    std::vector<Block> collectListBlocks(Block ref) const;
    std::vector<Block> collectHashedBlocks(Block ref, isize bucket) const;
    std::vector<Block> collectHashedBlocks(Block ref) const;


    //
    // Argument checkers
    //

public:

    void require_initialized() const;
    void require_formatted() const;
    void require_file_or_directory(const FSBlock &node) const;


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
