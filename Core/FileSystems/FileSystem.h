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
class FileSystem : public CoreObject {
    
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
    Block curr = 0;

    // Location of the bitmap blocks and extended bitmap blocks
    std::vector<Block> bmBlocks;
    std::vector<Block> bmExtBlocks;


    //
    // Initializing
    //
    
public:
    
    FileSystem() { };
    FileSystem(const MediaFile &file, isize part = 0) throws { init(file, part); }
    FileSystem(const ADFFile &adf) throws { init(adf); }
    FileSystem(const HDFFile &hdn, isize part = 0) throws { init(hdn, part); }
    FileSystem(FloppyDrive &dfn) throws { init(dfn); }
    FileSystem(const HardDrive &hdn, isize part = 0) throws { init(hdn, part); }

    virtual ~FileSystem();

    void init(FileSystemDescriptor layout, u8 *buf, isize len) throws;
    void init(const MediaFile &file, isize part) throws;
    void init(const ADFFile &adf) throws;
    void init(const HDFFile &hdn, isize part) throws;
    void init(FloppyDrive &dfn) throws;
    void init(const HardDrive &hdn, isize part) throws;

    bool initialized() const;
    bool formatted() const;


    //
    // Methods from CoreObject
    //
    
protected:
    
    const char *objectName() const override { return "FileSystem"; }
    void _dump(Category category, std::ostream &os) const override;


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
    FSBlock *read(Block nr) noexcept { return storage.read(nr); }
    FSBlock *read(Block nr, FSBlockType type) noexcept { return storage.read(nr, type); }
    FSBlock *read(Block nr, std::vector<FSBlockType> types) noexcept { return storage.read(nr, types); }
    const FSBlock *read(Block nr) const noexcept { return storage.read(nr); }
    const FSBlock *read(Block nr, FSBlockType type) const noexcept { return storage.read(nr, type); }
    const FSBlock *read(Block nr, std::vector<FSBlockType> types) const noexcept { return storage.read(nr, types); }

    // Returns a reference to a stored block
    FSBlock &at(Block nr) { return storage.at(nr); }
    FSBlock &at(Block nr, FSBlockType type) { return storage.at(nr, type); }
    FSBlock &at(Block nr, std::vector<FSBlockType> types) { return storage.at(nr, types); }
    const FSBlock &at(Block nr) const { return storage.at(nr); }
    const FSBlock &at(Block nr, FSBlockType type) const { return storage.at(nr, type); }
    const FSBlock &at(Block nr, std::vector<FSBlockType> types) const { return storage.at(nr, types); }

    // Operator overload
    FSBlock &operator[](size_t nr) { return storage[nr]; }
    const FSBlock &operator[](size_t nr) const { return storage[nr]; }

    // Returns the type of a certain block
    FSBlockType blockType(Block nr) const;

    // Checks block properties
    bool isEmpty(Block nr) const { return blockType(nr) == FSBlockType::EMPTY_BLOCK; }

    // Returns the usage type of a certain byte in a certain block
    FSItemType itemType(Block nr, isize pos) const;
    
    // Queries a pointer from the block storage (may return nullptr)
    FSBlock *blockPtr(Block nr) const;

    // Queries a pointer to a block of a certain type (may return nullptr)
    // FSBlock *bootBlockPtr(Block nr) const;
    // FSBlock *rootBlockPtr(Block nr) const;
    FSBlock *bitmapBlockPtr(Block nr) const;
    FSBlock *bitmapExtBlockPtr(Block nr) const;
    FSBlock *userDirBlockPtr(Block nr) const;
    FSBlock *fileHeaderBlockPtr(Block nr) const;
    FSBlock *fileListBlockPtr(Block nr) const;
    FSBlock *dataBlockPtr(Block nr) const;
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
    FSBlock *locateAllocationBit(Block nr, isize *byte, isize *bit) const;
    
    
    //
    // Managing files and directories
    //

public:

    // Returns the root of the directory tree
    FSBlock &root() { return at(rootBlock); }
    const FSBlock &root() const { return at(rootBlock); }

    // Returns the working directory
    FSBlock &pwd() { return at(curr); }
    const FSBlock &pwd() const { return at(curr); }

    // Returns the working directory
    FSBlock &parent(const FSBlock &node);
    const FSBlock &parent(const FSBlock &node) const;

    // Changes the working directory
    void cd(const FSName &name);
    void cd(const FSBlock &path);
    void cd(const string &path);

    // Returns a pointer to the parent directory block
    FSBlock *parentPtr(const FSBlock &root) const;

    // Seeks an item in the directory tree (returns nullptr if not found)
    FSBlock *seekPtr(const FSBlock &root, const FSName &name) const;
    FSBlock *seekPtr(const FSBlock &root, const FSString &name) const;
    FSBlock *seekPtr(const FSBlock &root, const std::vector<FSName> &name) const;
    FSBlock *seekPtr(const FSBlock &root, const std::vector<string> &name) const;
    FSBlock *seekPtr(const FSBlock &root, const fs::path &name) const;
    FSBlock *seekPtr(const FSBlock &root, const string &name) const;
    FSBlock *seekPtr(const FSBlock &root, const char *name) const;

    // Checks if a an item exists in the directory tree
    bool exists(const FSBlock &top, const fs::path &path) const;
    bool exists(const fs::path &path) const { return exists(pwd(), path); }

    // Lists the contents of a directory ('dir' command, 'list' command)
    void list(std::ostream &os, const FSBlock &path, const FSOpt &opt = {}) const;
    void list(std::ostream &os, const FSOpt &opt = {}) const { return list(os, pwd(), opt); }

    // Searches the directory tree ('find' command)
    std::vector<Block> find(const FSPattern &pattern, const FSOpt &opt = {}) const;
    void find(std::ostream &os, const FSBlock &path, const FSOpt &opt = {}) const;
    void find(std::ostream &os, const FSOpt &opt = {}) const { return list(os, pwd(), opt); }

    // Collects blocks of a certain type
    std::vector<Block> collectDataBlocks(Block ref) const;
    std::vector<FSBlock *> collectDataBlocks(const FSBlock &node) const;
    std::vector<Block> collectListBlocks(Block ref) const;
    std::vector<FSBlock *> collectListBlocks(const FSBlock &node) const;
    std::vector<Block> collectHashedBlocks(Block ref, isize bucket) const;
    std::vector<FSBlock *> collectHashedBlocks(const FSBlock &node, isize bucket) const;
    std::vector<Block> collectHashedBlocks(Block ref) const;
    std::vector<FSBlock *> collectHashedBlocks(const FSBlock &node) const;


    //
    // Integrity checking
    //

public:

    // Performs a sanity check
    bool verify();
    
    // Checks all blocks in this volume
    FSErrorReport check(bool strict);

    // Checks a single byte in a certain block
    Fault check(Block nr, isize pos, u8 *expected, bool strict) const;

    // Checks if the block with the given number is part of the volume
    bool isBlockNumber(isize nr) const { return nr >= 0 && nr < numBlocks(); }

    // Checks if the type of a block matches one of the provides types
    Fault checkBlockType(Block nr, FSBlockType type) const;
    Fault checkBlockType(Block nr, FSBlockType type, FSBlockType altType) const;

    // Checks if a certain block is corrupted
    bool isCorrupted(Block nr) const { return getCorrupted(nr) != 0; }

    // Returns the position in the corrupted block list (0 = OK)
    isize getCorrupted(Block nr) const;

    // Returns a reference to the next or the previous corrupted block
    Block nextCorrupted(Block nr) const;
    Block prevCorrupted(Block nr) const;

    // Checks if a certain block is the n-th corrupted block
    bool isCorrupted(Block nr, isize n) const;

    // Returns a reference to the n-th corrupted block
    Block seekCorruptedBlock(isize n) const;
    
    
    //
    // Traversing the file system
    //
    
public:

    // Creates a node tree resembling the directory structure
    FSTree traverse(const FSBlock &path, const FSOpt &opt = {}) const;

public:

    // Follows a linked list and collects all nodes
    std::vector<Block> collect(const Block nr, std::function<FSBlock *(FSBlock *)> next) const;
    std::vector<FSBlock *> collect(const FSBlock &node, std::function<FSBlock *(FSBlock *)> next) const;

private:

    // Main traversal code
    FSTree traverse(const FSBlock &path, const FSOpt &opt, std::unordered_set<Block> &visited) const;


    //
    // Traversing linked lists
    //
    
protected:
    
    // Returns the last element in the list of blocks with the same hash
    std::vector<Block> hashBlockChain(Block first) const;


    //
    // Argument checking helpers
    //

    void REQUIRE_INITIALIZED() const;
    void REQUIRE_FORMATTED() const;
    void REQUIRE_FILE_OR_DIRECTORY(FSBlock &node) const;


    //
    // GUI helper functions
    //

public:

    // Returns a block summary for creating the block usage image
    void analyzeBlockUsage(u8 *buffer, isize len) const;

    // Returns a usage summary for creating the block allocation image
    void analyzeBlockAllocation(u8 *buffer, isize len) const;

    // Returns a block summary for creating the diagnose image
    void analyzeBlockConsistency(u8 *buffer, isize len) const;
    
    // Searches the block list for a block of a specific type
    isize nextBlockOfType(FSBlockType type, isize after) const;

    // Searches the block list for a corrupted block
    isize nextCorruptedBlock(isize after) const;
};

}
