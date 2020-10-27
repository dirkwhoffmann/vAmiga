// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSHashTable::FSHashTable(FSVolume &ref) : volume(ref)
{
    memset(hashTable, 0, sizeof(hashTable));
}

void
FSHashTable::link(u32 ref)
{
    FSBlock *block = volume.block(ref);
    
    if (block) {
        
        // Compute the hash value for the new block
        u32 hashValue = volume.block(ref)->hashValue();
        
        // Add reference
        if (hashTable[hashValue] == 0) {
            hashTable[hashValue] = ref;
        } else {
            volume.block(hashTable[hashValue])->link(ref);
        }
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
    
    u32 ref = hashTable[hash];
    
    while (ref && volume.isBlockNumber(ref)) {

        FSBlock *block = volume.block(ref);
        
        // Return immediately if the item has been found
        if (block->matches(name)) return block;

        // If not, check the next item in the linked list
        ref = block->nextBlock();
    }
        
    return nullptr;
}

void
FSHashTable::write(u8 *ptr)
{
    for (long i = 0; i < 72; i++) {
        
        ptr[4 * i + 0] = BYTE3(hashTable[i]);
        ptr[4 * i + 1] = BYTE2(hashTable[i]);
        ptr[4 * i + 2] = BYTE1(hashTable[i]);
        ptr[4 * i + 3] = BYTE0(hashTable[i]);
    }
}
