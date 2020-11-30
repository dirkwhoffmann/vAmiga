/// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_PARTITION_H
#define _FS_PARTITION_H

#include "FSObjects.h"
#include "FSBlock.h"
#include "FSEmptyBlock.h"
#include "FSBootBlock.h"
#include "FSRootBlock.h"
#include "FSBitmapBlock.h"
#include "FSBitmapExtBlock.h"
#include "FSUserDirBlock.h"
#include "FSFileHeaderBlock.h"
#include "FSFileListBlock.h"
#include "FSDataBlock.h"

struct FSPartition : AmigaObject {
    
    // Cylinder boundaries
    u32 lowCyl = 0;
    u32 highCyl = 0;
    
    // Location of the root block
    u32 rootBlock = 0;
    
    // References to all bitmap blocks and bitmap extension blocks
    vector<u32> bmBlocks;
    vector<u32> bmExtBlocks;

    
    //
    // Initializing
    //
    
    FSPartition(u32 first, u32 last, u32 root);

    const char *getDescription() override { return "FSPartition"; }
    
    void dump();

    
    //
    // Querying partition properties
    //
    
    // Returns the number of cylinders in this partition
    u32 numCyls() { return highCyl - lowCyl + 1; }    
};

typedef std::vector<FSPartition> PTable;

#endif
