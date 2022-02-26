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
    
    //
    // Initializing
    //
    
//     FSPartition() { }
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "FSPartition"; }
    void _dump(dump::Category category, std::ostream& os) const override { };
    

};

typedef FSPartition* FSPartitionPtr;
