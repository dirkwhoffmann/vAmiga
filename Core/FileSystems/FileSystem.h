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
 * See also: MutableFileSystem
 */
class FileSystem : public CoreObject, public Inspectable<FSInfo, FSStats> {

    friend struct FSBlock;
    friend struct FSHashTable;
    friend struct FSPartition;
    friend class DiskDoctor;

public:

    // Analyzer and rectifier
    DiskDoctor doctor = DiskDoctor(*this);

protected:

    // File system information
    FSTraits traits;

    // File system version
    FSVolumeType dos = FSVolumeType::NODOS;

    // Block storage
    BlockStorage storage = BlockStorage(this);

    // Size of a single block in bytes
    isize bsize = 512;

    // Number of reserved blocks
    isize numReserved = 0;

    // Location of the root block
    Block rootBlock = 0;

    // The current directory (points to the root block by default)
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
    FileSystem(const HDFFile &hdn, isize part = 0) : FileSystem() { init(hdn, part); }
    FileSystem(FloppyDrive &dfn) : FileSystem() { init(dfn); }
    FileSystem(const HardDrive &hdn, isize part = 0) : FileSystem() { init(hdn, part); }

    virtual ~FileSystem();

    void init(FileSystemDescriptor layout, u8 *buf, isize len) throws;
    void init(const MediaFile &file, isize part) throws;
    void init(const ADFFile &adf) throws;
    void init(const HDFFile &hdn, isize part) throws;
    void init(FloppyDrive &dfn) throws;
    void init(const HardDrive &hdn, isize part) throws;

    bool isInitialized() const;
    bool isFormatted() const;


    //
    // Methods from CoreObject
    //
    
protected:
    
    const char *objectName() const override { return "FileSystem"; }
    void _dump(Category category, std::ostream &os) const override;


    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(FSInfo &result) const override;
    void cacheStats(FSStats &result) const override;


    //
    // Querying file system properties
    //

public:

    FSTraits &getTraits();

    // Returns capacity information
    isize numBlocks() const { return storage.numBlocks(); }
    isize numBytes() const { return storage.numBytes(); }
    isize blockSize() const { return storage.blockSize(); }

    // Reports usage information
    isize freeBlocks() const;
    isize usedBlocks() const;
    isize freeBytes() const { return freeBlocks() * bsize; }
    isize usedBytes() const { return usedBlocks() * bsize; }
    double fillLevel() const { return double(100) * usedBlocks() / numBlocks(); }
    
    // Returns the DOS version
    FSVolumeType getDos() const { return dos; }
    bool isOFS() const { return isOFSVolumeType(dos); }
    bool isFFS() const { return isFFSVolumeType(dos); }
    bool isINTL() const { return isINTLVolumeType(dos); }

    // Reads information from the root block
    FSName getName() const;
    string getCreationDate() const;
    string getModificationDate() const;
    
    // Analyzes the boot block
    string getBootBlockName() const;
    BootBlockType bootBlockType() const;
    bool hasVirus() const { return bootBlockType() == BootBlockType::VIRUS; }

    
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
    FSBlockType blockType(Block nr) const;

    // Checks block properties
    bool isEmpty(Block nr) const { return blockType(nr) == FSBlockType::EMPTY_BLOCK; }

    // Returns the usage type of a certain byte in a certain block
    FSItemType itemType(Block nr, isize pos) const;
    
    // Queries a pointer from the block storage (may return nullptr)
    FSBlock *blockPtr(Block nr) const; // DEPRECATED

    // Queries a pointer to a block of a certain type (may return nullptr)
    FSBlock *hashableBlockPtr(Block nr) const;
    
    // Reads a single byte from a block
    u8 readByte(Block nr, isize offset) const;

    // Returns a portion of the block as an ASCII dump
    string ascii(Block nr, isize offset, isize len) const;
    
    // Predicts the type of a block by analyzing its number and data
    FSBlockType predictBlockType(Block nr, const u8 *buf) const;


    //
    // Querying the block allocation bitmap
    //

public:
    
    // Checks if a block is free or allocated
    bool isFree(Block nr) const;
    bool isAllocated(Block nr) const { return !isFree(nr); }
    
protected:
    
    // Locates the allocation bit for a certain block
    FSBlock *locateAllocationBit(Block nr, isize *byte, isize *bit);
    const FSBlock *locateAllocationBit(Block nr, isize *byte, isize *bit) const;

    
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
    FSBlock *parent(const FSBlock *node);
    const FSBlock &parent(const FSBlock &node) const;
    const FSBlock *parent(const FSBlock *node) const;

    // Changes the working directory
    void cd(const FSName &name);
    void cd(const FSBlock &path);
    void cd(const string &path);

    // Checks if a an item exists in the directory tree
    bool exists(const FSBlock &top, const fs::path &path) const;
    bool exists(const fs::path &path) const { return exists(pwd(), path); }

    // Seeks an item in the directory tree (returns nullptr if not found)
    FSBlock *seekPtr(const FSBlock *root, const FSName &name);
    FSBlock *seekPtr(const FSBlock *root, const fs::path &name);
    FSBlock *seekPtr(const FSBlock *root, const string &name);
    const FSBlock *seekPtr(const FSBlock *root, const FSName &name) const;
    const FSBlock *seekPtr(const FSBlock *root, const fs::path &name) const;
    const FSBlock *seekPtr(const FSBlock *root, const string &name) const;

    // Seeks an item in the directory tree (returns nullptr if not found)
    FSBlock &seek(const FSBlock &root, const FSName &name);
    FSBlock &seek(const FSBlock &root, const fs::path &name);
    FSBlock &seek(const FSBlock &root, const string &name);
    const FSBlock &seek(const FSBlock &root, const FSName &name) const;
    const FSBlock &seek(const FSBlock &root, const fs::path &name) const;
    const FSBlock &seek(const FSBlock &root, const string &name) const;

    // Seeks all items satisfying a predicate
    std::vector<FSBlock *> find(const FSPattern &pattern) const;
    std::vector<FSBlock *> find(const FSBlock *root, const FSOpt &opt) const;
    std::vector<FSBlock *> find(const FSBlock &root, const FSOpt &opt) const;
    std::vector<Block> find(const Block root, const FSOpt &opt) const;

    // Seeks all items with a pattern-matching name
    std::vector<FSBlock *> find(const FSBlock *root, const FSPattern &pattern) const;
    std::vector<FSBlock *> find(const FSBlock &root, const FSPattern &pattern) const;
    std::vector<Block> find(const Block root, const FSPattern &pattern) const;

    // Collects all items with a pattern-matching path
    std::vector<FSBlock *> match(const FSBlock *root, const FSPattern &pattern) const;
    std::vector<FSBlock *> match(const FSBlock &root, const FSPattern &pattern) const;
    std::vector<FSBlock *> match(const FSBlock *root, std::vector<FSPattern> pattern) const;

    // Lists the contents of a directory ('dir' command, 'list' command)
    void list(std::ostream &os, const FSBlock &path, const FSOpt &opt = {}) const;
    void list(std::ostream &os, const FSOpt &opt = {}) const { return list(os, pwd(), opt); }
    void listDirectory(std::ostream &os, const FSBlock &path, const FSOpt &opt = {}) const;
    void listItems(std::ostream &os, std::vector<FSBlock *> items, const FSOpt &opt = {}) const;

private:

    std::vector<FSBlock *> find(const FSBlock *root, const FSOpt &opt,
                                std::unordered_set<Block> &visited) const;

    //
    // Integrity checking
    //

public:

    // Performs a sanity check
    bool verify();
    
    // Checks a single byte in a certain block
    Fault check(Block nr, isize pos, u8 *expected, bool strict) const;

    // Checks if the block with the given number is part of the volume
    bool isBlockNumber(isize nr) const { return nr >= 0 && nr < numBlocks(); }

    // Checks if the type of a block matches one of the provides types
    Fault checkBlockType(Block nr, FSBlockType type) const;
    Fault checkBlockType(Block nr, FSBlockType type, FSBlockType altType) const;

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
    // Traversing linked lists
    //
    
protected:
    
    // Returns
    // TODO: DEPRECATE
    std::vector<Block> hashBlockChain(Block first) const;


    //
    // Argument checking helpers
    //

    // TODO: Prettify and use
    void REQUIRE_INITIALIZED() const;
    void REQUIRE_FORMATTED() const;
    void REQUIRE_FILE_OR_DIRECTORY(FSBlock &node) const;


    //
    // GUI helper functions
    //

public:

    // Returns a block summary for creating the block usage image
    void createUsageMap(u8 *buffer, isize len);

    // Returns a usage summary for creating the block allocation image
    void createAllocationMap(u8 *buffer, isize len);

    // Returns a block summary for creating the diagnose image
    void createHealthMap(u8 *buffer, isize len);
    
    // Searches the block list for a block of a specific type
    isize nextBlockOfType(FSBlockType type, isize after) const;
};

}
