// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "FSBlock.h"
#include "FSDescriptors.h"
#include "FSObjects.h"
#include "ADFFile.h"
#include "HDFFile.h"
#include <stack>
#include <set>

class ADFFile;
class HDFFile;
class Drive;
class HardDrive;

class FileSystem : public AmigaObject {
    
    friend struct FSBlock;
    friend struct FSHashTable;
    friend struct FSPartition;
    
protected:
                        
    // File system version
    FSVolumeType dos = FS_NODOS;
    
    // Block storage
    std::vector<BlockPtr> blocks;
            
    // Size of a single block in bytes
    isize bsize = 0;

    // Number of reserved blocks
    isize numReserved = 0;

    // Location of the root block
    Block rootBlock = 0;
    
    // Location of the bitmap blocks and extended bitmap blocks
    std::vector<Block> bmBlocks;
    std::vector<Block> bmExtBlocks;
        
    // The currently selected directory (reference to FSDirBlock)
    Block cd = 0;
    
    
    //
    // Methods from AmigaObject
    //
    
protected:
    
    const char *getDescription() const override { return "FileSystem"; }
    void _dump(dump::Category category, std::ostream& os) const override;

    
    //
    // Querying layout properties
    //
    
public:
                
    // Returns the capacity of this volume
    isize numBlocks() const { return isize(blocks.size()); }
    isize numBytes() const { return numBlocks() * bsize; }
    
    // Reports usage information
    isize freeBlocks() const;
    isize usedBlocks() const;
    isize freeBytes() const;
    isize usedBytes() const;

    
    //
    // Querying file system properties
    //
    
public:
    
    // Returns the DOS version
    FSVolumeType getDos() const { return dos; }
    bool isOFS() const { return isOFSVolumeType(dos); }
    bool isFFS() const { return isFFSVolumeType(dos); }

    // Reads the volume name from the root block
    FSName getName() const;
    
    
    //
    // Accessing blocks
    //
    
public:
    
    // Returns the type of a certain block
    FSBlockType blockType(Block nr) const;

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
    // Integrity checking
    //

public:
    
    // Checks all blocks in this volume
    FSErrorReport check(bool strict) const;

    // Checks a single byte in a certain block
    ErrorCode check(Block nr, isize pos, u8 *expected, bool strict) const;

    // Checks if the block with the given number is part of the volume
    bool isBlockNumber(isize nr) const { return nr >= 0 && nr < numBlocks(); }

    // Checks if the type of a block matches one of the provides types
    ErrorCode checkBlockType(Block nr, FSBlockType type) const;
    ErrorCode checkBlockType(Block nr, FSBlockType type, FSBlockType altType) const;

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
    // Traversing the file system
    //
    
protected:
    
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
    // Traversing linked lists
    //
    
protected:
    
    // Returns the last element in the list of extension blocks
    FSBlock *lastFileListBlockInChain(Block start);
    FSBlock *lastFileListBlockInChain(FSBlock *block);
    
    // Returns the last element in the list of blocks with the same hash
    FSBlock *lastHashBlockInChain(Block start);
    FSBlock *lastHashBlockInChain(FSBlock *block);
    
 
    //
    // Importing and exporting
    //
    
public:
        
    // Predicts the type of a block by analyzing its number and data
    FSBlockType predictBlockType(Block nr, const u8 *buffer);

    
    // TODO: MOVE ALL NON-WRITE-RELATED FUNCTIONS TO THIS CLASS
};
