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
FSHashTable::write(u8 *ptr)
{
    for (long i = 0; i < 72; i++) {
        
        if (hashTable[i] == nullptr) continue;
        
        printf("hashValue[%d] = %d\n", i, hashTable[i]->nr);
        
        ptr[4 * i + 0] = BYTE3(hashTable[i]->nr);
        ptr[4 * i + 1] = BYTE2(hashTable[i]->nr);
        ptr[4 * i + 2] = BYTE1(hashTable[i]->nr);
        ptr[4 * i + 3] = BYTE0(hashTable[i]->nr);
    }
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
