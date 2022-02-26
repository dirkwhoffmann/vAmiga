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

    
    
    
    // TODO: MOVE ALL NON-WRITE-RELATED FUNCTIONS TO THIS CLASS
};
