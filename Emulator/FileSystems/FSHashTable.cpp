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
    hashTableSize = (volume.bsize / sizeof(u32)) - 56;
    
    hashTable = new u32[hashTableSize]();
}

FSHashTable::~FSHashTable()
{
    delete [] hashTable;
}

void
FSHashTable::dump()
{
    for (int i = 0; i < hashTableSize; i++) {
        if (hashTable[i]) printf("%d:%d ", i, hashTable[i]);
    }
}

bool
FSHashTable::link(u32 ref)
{
    FSBlock *block = volume.block(ref);

    return block ? link(ref, volume.block(ref)->hashValue()) : false;
}

bool
FSHashTable::link(FSBlock *block)
{
    return block ? link(block->nr, block->hashValue()) : false;
}

bool
FSHashTable::link(u32 ref, u32 hashValue)
{
    assert(hashValue < hashTableSize);
    
    if (!volume.isBlockNumber(ref)) return false;
    
    if (hashTable[hashValue] == 0) {
        hashTable[hashValue] = ref;
    } else {
        volume.block(hashTable[hashValue])->setNext(ref);
    }
    return true;
}

FSBlock *
FSHashTable::seek(FSName name)
{
    u32 ref = hashTable[name.hashValue()];
    
    while (ref && volume.isBlockNumber(ref)) {

        FSBlock *block = volume.block(ref);
        
        // Return the block reference if the item has been found
        if (block->matches(name)) return block;

        // If not, check the next item in the linked list
        ref = block->getNext();
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
