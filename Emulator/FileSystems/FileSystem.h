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
    // void _dump(dump::Category category, std::ostream& os) const override;

    
    //
    // Querying layout properties
    //
    
public:
                
    // Returns the capacity of this volume
    isize numBlocks() const { return isize(blocks.size()); }
    isize numBytes() const { return numBlocks() * bsize; }
    
    
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
    FSBlock *bootBlockPtr(Block nr) const;
    FSBlock *rootBlockPtr(Block nr) const;
    FSBlock *bitmapBlockPtr(Block nr) const;
    FSBlock *bitmapExtBlockPtr(Block nr) const;
    FSBlock *userDirBlockPtr(Block nr) const;
    FSBlock *fileHeaderBlockPtr(Block nr) const;
    FSBlock *fileListBlockPtr(Block nr) const;
    FSBlock *dataBlockPtr(Block nr) const;
    FSBlock *hashableBlockPtr(Block nr) const;
    
    
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
    
    bool isBlockNumber(isize nr) const { return nr >= 0 && nr < numBlocks(); }

    
    // TODO: MOVE ALL NON-WRITE-RELATED FUNCTIONS TO THIS CLASS
};
