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
#include "FSName.h"

struct FSHashTable {

    static const int hasTableSize = 72;
    class HashableBlock *hashTable[hasTableSize];
    
    //
    // Methods
    //
    
    FSHashTable();
    
    // Adds a new reference to the hash table
    void link(HashableBlock *block);

    // Seeks for a match inside the hash table
    HashableBlock *seek(FSName name);
    HashableBlock *seek(FSName name, u32 hash);

    // Exports the block in AmigaDOS format
    void write(u8 *ptr);    
};

#endif
