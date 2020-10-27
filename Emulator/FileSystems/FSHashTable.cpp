// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSHashTable::FSHashTable()
{
    memset(hashTable, 0, sizeof(hashTable));
}

void
FSHashTable::link(FSBlock *block)
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

FSBlock *
FSHashTable::seek(FSName name)
{
    return seek(name, name.hashValue());
}

FSBlock *
FSHashTable::seek(FSName name, u32 hash)
{
    assert(hash < hasTableSize);
    
    FSBlock *block = hashTable[hash];
    
    while (block) {

        // Return immediately if the item has been found
        if (block->matches(name)) return block;

        // If not, check the next item in the linked list
        block = block->nextBlock();
    }
        
    return nullptr;
}

void
FSHashTable::write(u8 *ptr)
{
    for (long i = 0; i < 72; i++) {
        
        if (hashTable[i] == nullptr) continue;
                
        ptr[4 * i + 0] = BYTE3(hashTable[i]->nr);
        ptr[4 * i + 1] = BYTE2(hashTable[i]->nr);
        ptr[4 * i + 2] = BYTE1(hashTable[i]->nr);
        ptr[4 * i + 3] = BYTE0(hashTable[i]->nr);
    }
}
