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

    // The volume this table belongs to
    class FSVolume &volume;

    // Hash table size
    static const size_t hashTableSize = 72;

    // Hash table entries
    u32 hashTable[hashTableSize];
    
    
    //
    // Methods
    //
    
    FSHashTable(FSVolume &ref);
    
    void dump();
    
    // Adds a new reference to the hash table
    bool link(u32 ref);
    bool link(class FSBlock *block);
    bool link(u32 ref, u32 hashValue);

    // Looks for a matching item inside the hash table
    class FSBlock *seek(FSName name);
    
    // Exports the block in AmigaDOS format
    void write(u8 *ptr);    
};

#endif
