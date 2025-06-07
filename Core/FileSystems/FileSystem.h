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
#include "FSPath.h"
#include "ADFFile.h"
#include "HDFFile.h"
#include <stack>
#include <set>

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
    
protected:

    // File system information
    FSTraits traits;

    // File system version
    FSVolumeType dos = FSVolumeType::NODOS;
    
    // Block storage
    std::vector<BlockPtr> blocks;

    // Size of a single block in bytes
    isize bsize = 512;

    // Number of reserved blocks
    isize numReserved = 0;

    // Location of the root block
    Block rootBlock = 0;
    
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
    
protected:

    void init(const MediaFile &file, isize part) throws;
    void init(const ADFFile &adf) throws;
    void init(const HDFFile &hdn, isize part) throws;
    void init(FloppyDrive &dfn) throws;
    void init(const HardDrive &hdn, isize part) throws;

    void init(FileSystemDescriptor layout, u8 *buf, isize len) throws;

    
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
    isize numBlocks() const { return isize(blocks.size()); }
    isize numBytes() const { return numBlocks() * bsize; }
    isize blockSize() const { return bsize; }

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

    // Reads information from the root block
    FSName getName() const;
    string getCreationDate() const;
    string getModificationDate() const;
    
    // Analyzes the boot block
    string getBootBlockName() const;
    BootBlockType bootBlockType() const;
    bool hasVirus() const { return bootBlockType() == BootBlockType::VIRUS; }

    
    //
    // Accessing blocks
    //
    
public:
    
    // Returns the type of a certain block
    FSBlockType blockType(Block nr) const;

    // Checks block properties
    bool isEmpty(Block nr) const { return blockType(nr) == FSBlockType::EMPTY_BLOCK; }

    // Returns the usage type of a certain byte in a certain block
    FSItemType itemType(Block nr, isize pos) const;
    
    // Queries a pointer from the block storage (may return nullptr)
    FSBlock *blockPtr(Block nr) const;

    // Queries a pointer to a block of a certain type (may return nullptr)
    FSBlock *bootBlockPtr(Block nr) const;
    FSBlock *rootBlockPtr(Block nr) const;
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
    FSPath rootDir() const;

    // Seeks an item in the directory tree
    FSPath seek(const FSPath &top, const fs::path &path) const { return top.seek(path); }
    FSPath seekDir(const FSPath &top, const fs::path &path) const { return top.seekDir(path); }
    FSPath seekFile(const FSPath &top, const fs::path &path) const { return top.seekFile(path); }

    // Seeks an item in the directory tree starting from the root
    FSPath seek(const fs::path &path) const { return seek(rootDir(), path); }
    FSPath seekDir(const fs::path &path) const { return seekDir(rootDir(), path); }
    FSPath seekFile(const fs::path &path) const { return seekFile(rootDir(), path); }
    FSPath cd(const fs::path &path) const { return seekDir(path); }

    // Checks if a an item exists in the directory tree
    bool exists(const FSPath &top, const fs::path &path) const;
    bool exists(const fs::path &path) const { return exists(rootDir(), path); }

    // Prints a directory listing
    void printDirectory(bool recursive) const throws;
    

    //
    // Integrity checking
    //

public:

    // Performs a sanity check
    bool verify() const;
    
    // Checks all blocks in this volume
    FSErrorReport check(bool strict) const;

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
    
protected:
    
    // Returns a collections of nodes for all items in a directory
    void collect(const FSPath &path, std::vector<Block> &list, bool recursive = true) const throws;

private:
    
    // Collects all references stored in a hash table
    void collectHashedRefs(Block nr, std::stack<Block> &list,
                           std::set<Block> &visited) const throws;

    // Collects all references with the same hash value
    void collectRefsWithSameHashValue(Block nr, std::stack<Block> &list,
                                      std::set<Block> &visited) const throws;

    
    //
    // Traversing linked lists
    //
    
protected:
    
    // Returns the last element in the list of extension blocks
    FSBlock *lastFileListBlockInChain(Block start) const;
    FSBlock *lastFileListBlockInChain(FSBlock *block) const;
    
    // Returns the last element in the list of blocks with the same hash
    FSBlock *lastHashBlockInChain(Block start) const;
    FSBlock *lastHashBlockInChain(FSBlock *block) const;
    

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
