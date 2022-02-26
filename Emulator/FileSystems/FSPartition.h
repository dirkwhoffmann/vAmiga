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
#include "FSDescriptors.h"

struct FSPartition : AmigaObject {
    
    // The device this partition is part of
    class FSDevice &dev;
                
    // Location of the root block
    Block rootBlock = 0;
    
    // Location of the bitmap blocks and extended bitmap blocks
    std::vector<Block> bmBlocks;
    std::vector<Block> bmExtBlocks;

    
    //
    // Initializing
    //
    
    FSPartition(FSDevice &ref) : dev(ref) { }
    FSPartition(FSDevice &ref, FSDeviceDescriptor &layout);
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "FSPartition"; }
    void _dump(dump::Category category, std::ostream& os) const override { };
    

};

typedef FSPartition* FSPartitionPtr;
