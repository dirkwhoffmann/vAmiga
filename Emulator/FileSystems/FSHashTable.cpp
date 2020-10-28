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
        volume.block(hashTable[hashValue])->link(ref);
    }
    return true;
}

/*
FSBlock *
FSHashTable::seek(FSName name)
{
    return seek(name, name.hashValue());
}
*/

FSBlock *
FSHashTable::seek(FSName name)
{
    u32 ref = hashTable[name.hashValue()];
    
    while (ref && volume.isBlockNumber(ref)) {

        FSBlock *block = volume.block(ref);
        
        // Return immediately if the item has been found
        if (block->matches(name)) return block;

        // If not, check the next item in the linked list
        ref = block->nextBlock();
    }
        
    return nullptr;
}

bool
FSHashTable::check()
{
    bool result = true;
    
    for (int i = 0; i < hashTableSize; i++) {

        if (hashTable[i] == 0) continue;
        
        FSBlock *block = volume.block(hashTable[i]);
        
        if (block) {
            
            FSBlockType type = block->type();
            if (type != FS_USERDIR_BLOCK && type != FS_FILEHEADER_BLOCK) {

                printf("Hash table [%d]: Referenced block has invalid type %d\n", i, type);
                result = false;
            }

        } else {
            
            printf("Hash table [%d]: Entry is not a block reference\n", i);
        }
    }
    
    return result;
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
