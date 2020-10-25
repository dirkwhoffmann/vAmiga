// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSHashTable.h"
#include "FSBlocks.h"
#include "Utils.h"

FSHashTable::FSHashTable()
{
    memset(hashTable, 0, sizeof(hashTable));
}

void
FSHashTable::link(HashableBlock *block)
{
    assert(block != nullptr);
    
    // Determine hash value
    u32 hashValue = block->hashValue();
    assert(hashValue < 72);
    
    // Add reference
    if (hashTable[hashValue] != nullptr) {
        hashTable[hashValue]->link(block);
    } else {
        hashTable[hashValue] = block;
    }
}
