// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSObjects.h"
#include "FSPartition.h"
#include "FSBlock.h"
#include "ADFFile.h"
#include "HDFFile.h"
#include <stack>
#include <set>

/* This class provides the basic functionality of the Amiga File Systems OFS
 * and FFS. Starting from an empty volume, files can be added or removed,
 * and boot blocks can be installed. Furthermore, functionality is provided to
 * import and export the file system from and to ADF files.
 */

class FSDevice : public AmigaObject {
    
    friend struct FSBlock;
    friend struct FSHashTable;
    friend struct FSPartition;

protected:
            
    // Physical device parameters
    isize numCyls = 0;
    isize numHeads = 0;
    
    // Logical device parameters
    isize numSectors = 0;
    i64 numBlocks = 0;
    isize numReserved = 0;
    isize bsize = 0;
            
    // The partition table
    std::vector<FSPartitionPtr> partitions;
    
    // The block storage
    std::vector<BlockPtr> blocks;
            
    // The currently selected partition
    isize cp = 0;
    
    // The currently selected directory (reference to FSDirBlock)
    Block cd = 0;
    
    
    //
    // Initializing
    //
    
public:

    FSDevice(isize capacity) { init(capacity); }
    FSDevice(FSDeviceDescriptor &layout) { init(layout); }
    FSDevice(DiskDiameter dia, DiskDensity den) { init(dia, den); }
    FSDevice(DiskDiameter dia, DiskDensity den, const string &path) { init(dia, den, path); }
    FSDevice(class ADFFile &adf) throws { init(adf); }
    FSDevice(class HDFFile &hdf) throws { init(hdf); }
    FSDevice(FSVolumeType type, const string &path) { init(type, path); }
    ~FSDevice();
    
private:
    
    void init(isize capacity);
    void init(FSDeviceDescriptor &layout);
    void init(DiskDiameter type, DiskDensity density);
    void init(DiskDiameter type, DiskDensity density, const string &path);
    void init(class ADFFile &adf) throws;
    void init(class HDFFile &hdf) throws;
    void init(FSVolumeType type, const string &path);

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "FSVolume"; }
    void _dump(dump::Category category, std::ostream& os) const override;
    

    //
    // Analyzing
    //
    
public:
    
    // Prints information about this volume
    void info();
            
    
    //
    // Querying file system properties
    //
    
public:
            
    // Returns the total device capacity in blocks
    i64 getCapacity() { return numBlocks; }
    
    // Reports layout information
    isize getNumCyls() { return numCyls; }
    isize getNumHeads() { return numHeads; }
    isize getNumTracks() { return getNumCyls() * getNumHeads(); }
    isize getNumSectors() { return numSectors; }
    i64 getNumBlocks() { return numBlocks; }

    
    //
    // Querying properties of the current partition
    //
    
    // Returns the DOS version of the current partition
    FSVolumeType dos() const { return partitions[cp]->dos; }
    bool isOFS() const { return partitions[cp]->isOFS(); }
    bool isFFS() const { return partitions[cp]->isFFS(); }
    
    
    //
    // Working with partitions
    //
    
public:
    
    // Returns the number of partitions
    isize numPartitions() { return (isize)partitions.size(); }
    
    // Returns the partition a certain block belongs to
    isize partitionForBlock(Block nr);

    // Gets or sets the name of the current partition
    FSName getName() { return partitions[cp]->getName(); }
    void setName(FSName name) { partitions[cp]->setName(name); }
    
    
    //
    // Working with boot blocks
    //
    
public:
    // Installs a boot block
    void makeBootable(BootBlockId id) { partitions[cp]->makeBootable(id); }

    // Removes a boot block virus from the current partition (if any)
    void killVirus() { partitions[cp]->killVirus(); }
    
    
    //
    // Accessing blocks
    //
    
public:
    
    // Returns the type of a certain block
    FSBlockType blockType(Block nr);

    // Returns the usage type of a certain byte in a certain block
    FSItemType itemType(Block nr, isize pos) const;
    
    // Queries a pointer from the block storage (may return nullptr)
    FSBlock *blockPtr(Block nr) const;

    // Queries a pointer to a block of a certain type (may return nullptr)
    FSBlock *bootBlockPtr(Block nr);
    FSBlock *rootBlockPtr(Block nr);
    FSBlock *bitmapBlockPtr(Block nr);
    FSBlock *bitmapExtBlockPtr(Block nr);
    FSBlock *userDirBlockPtr(Block nr);
    FSBlock *fileHeaderBlockPtr(Block nr);
    FSBlock *fileListBlockPtr(Block nr);
    FSBlock *dataBlockPtr(Block nr);
    FSBlock *hashableBlockPtr(Block nr);
    
    
    //
    // Creating and deleting blocks
    //
    
public:
    
    // Updates the checksums in all blocks
    void updateChecksums();
    
    
    //
    // Managing directories and files
    //
    
public:
    
    // Returns the block representing the current directory
    FSBlock *currentDirBlock();
    
    // Changes the current directory
    FSBlock *changeDir(const string &name);

    // Prints a directory listing
    void printDirectory(bool recursive) throws;

    // Returns the path of a file system item
    string getPath(FSBlock *block);
    string getPath(Block nr) { return getPath(blockPtr(nr)); }
    string getPath() { return getPath(currentDirBlock()); }

    // Seeks an item inside the current directory
    Block seekRef(FSName name);
    Block seekRef(const string &name) { return seekRef(FSName(name)); }
    FSBlock *seek(const string &name) { return blockPtr(seekRef(name)); }
    FSBlock *seekDir(const string &name) { return userDirBlockPtr(seekRef(name)); }
    FSBlock *seekFile(const string &name) { return fileHeaderBlockPtr(seekRef(name)); }

    // Adds a reference to the current directory
    void addHashRef(Block nr);
    void addHashRef(FSBlock *block);
    
    // Creates a new directory
    FSBlock *createDir(const string &name);

    // Creates a new file
    FSBlock *createFile(const string &name);
    FSBlock *createFile(const string &name, const u8 *buffer, isize size);
    FSBlock *createFile(const string &name, const string &str);
        
    
    //
    // Traversing linked lists
    //
    
    // Returns the last element in the list of extension blocks
    FSBlock *lastFileListBlockInChain(Block start);
    FSBlock *lastFileListBlockInChain(FSBlock *block);

    // Returns the last element in the list of blocks with the same hash
    FSBlock *lastHashBlockInChain(Block start);
    FSBlock *lastHashBlockInChain(FSBlock *block);

    
    //
    // Traversing the file system
    //
    
public:
    
    // Returns a collections of nodes for all items in the current directory
    void collect(Block nr, std::vector<Block> &list, bool recursive = true) throws;
    
private:
    
    // Collects all references stored in a hash table
    void collectHashedRefs(Block nr, std::stack<Block> &list,
                           std::set<Block> &visited) throws;
    
    // Collects all references with the same hash value
    void collectRefsWithSameHashValue(Block nr, std::stack<Block> &list,
                                      std::set<Block> &visited) throws;

 
    //
    // Integrity checking
    //

public:
    
    // Checks all blocks in this volume
    FSErrorReport check(bool strict) const;

    // Checks a single byte in a certain block
    ErrorCode check(Block nr, isize pos, u8 *expected, bool strict) const;

    // Checks if the block with the given number is part of the volume
    bool isBlockNumber(isize nr) { return nr < numBlocks; }

    // Checks if the type of a block matches one of the provides types
    ErrorCode checkBlockType(Block nr, FSBlockType type);
    ErrorCode checkBlockType(Block nr, FSBlockType type, FSBlockType altType);

    // Checks if a certain block is corrupted
    bool isCorrupted(Block nr) { return getCorrupted(nr) != 0; }

    // Returns the position in the corrupted block list (0 = OK)
    isize getCorrupted(Block nr);

    // Returns a reference to the next or the previous corrupted block
    Block nextCorrupted(Block nr);
    Block prevCorrupted(Block nr);

    // Checks if a certain block is the n-th corrupted block
    bool isCorrupted(Block nr, isize n);

    // Returns a reference to the n-th corrupted block
    Block seekCorruptedBlock(isize n);
    
    
    //
    // Importing and exporting
    //
    
public:
        
    // Reads a single byte from a block
    u8 readByte(Block nr, isize offset) const;

    // Predicts the type of a block by analyzing its number and data (DEPRECATED)
    FSBlockType predictBlockType(Block nr, const u8 *buffer);

    // Imports the volume from a buffer compatible with the ADF format
    void importVolume(const u8 *src, isize size) throws;

    // Imports a directory from the host file system
    void importDirectory(const string &path, bool recursive = true) throws;
    void importDirectory(const fs::directory_entry &dir, bool recursive) throws;
    
    // Exports the volume to a buffer compatible with the ADF format
    bool exportVolume(u8 *dst, isize size);
    bool exportVolume(u8 *dst, isize size, ErrorCode *error);

    // Exports a single block or a range of blocks
    bool exportBlock(Block nr, u8 *dst, isize size);
    bool exportBlock(Block nr, u8 *dst, isize size, ErrorCode *error);
    bool exportBlocks(Block first, Block last, u8 *dst, isize size);
    bool exportBlocks(Block first, Block last, u8 *dst, isize size, ErrorCode *error);

    // Exports the volume to a directory of the host file system
    void exportDirectory(const string &path) throws;
};
