// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_HASHTABLE_H
#define _FS_HASHTABLE_H

#include "Aliases.h"

class FSHashTable {
        
    class HashableBlock *hashTable[72];
    
public:

    FSHashTable();
    
    void write(u8 *ptr);    
    void link(HashableBlock *block);
};

#endif
