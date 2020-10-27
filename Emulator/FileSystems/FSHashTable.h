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

    // The volume this block belongs to
    class FSVolume &volume;

    // Hash table size
    static const int hasTableSize = 72;

    // Hash table entries
    u32 hashTable[hasTableSize];
    
    
    //
    // Methods
    //
    
    FSHashTable(FSVolume &ref);
    
    // Adds a new reference to the hash table
    void link(u32 ref);

    // Seeks for a match inside the hash table
    class FSBlock *seek(FSName name);
    class FSBlock *seek(FSName name, u32 hash);

    // Exports the block in AmigaDOS format
    void write(u8 *ptr);    
};

#endif
