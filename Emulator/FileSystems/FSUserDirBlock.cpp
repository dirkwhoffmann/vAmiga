// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSUserDirBlock::FSUserDirBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
}

FSUserDirBlock::FSUserDirBlock(FSVolume &ref, u32 nr, const char *name) : FSUserDirBlock(ref, nr)
{
    this->name = FSName(name);
}

void
FSUserDirBlock::printName()
{
    printf("%s", name.name);
}

void
FSUserDirBlock::printPath()
{
    FSBlock *ref = volume.block(getParent());
    if (ref) ref->printPath();
    printf("/");
    printName();
}

void
FSUserDirBlock::dump()
{
    printf("        Name: "); printName(); printf("\n");
    printf("        Path: "); printPath(); printf("\n");
    printf("     Comment: "); comment.dump(); printf("\n");
    printf("     Created: "); created.dump(); printf("\n");
    printf("  Hash table: "); hashTable.dump(); printf("\n");
    printf("      Parent: %d\n", parent);
    printf("        Next: %d\n", next);
}

bool
FSUserDirBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);

    for (int i = 0; i < hashTable.hashTableSize; i++) {
       
        u32 ref = hashTable.hashTable[i];
        if (ref == 0) continue;
        
        result &= assertInRange(ref, verbose);
        result &= assertHasType(ref, FS_USERDIR_BLOCK, FS_FILEHEADER_BLOCK);
    }

    return result;
}

void
FSUserDirBlock::write(u8 *p)
{
    // Start from scratch
    memset(p, 0, 512);
    
    // Type
    p[3] = 0x02;
    
    // Block pointer to itself
    write32(p + 4, nr);
    
    // Hashtable
    hashTable.write(p + 24);

    // Protection status bits
    u32 protection = 0;
    write32(p + 320, protection);
    
    // Comment as BCPL string
    comment.write(p + 328);
    
    // Creation date
    created.write(p + 420);
    
    // Directory name as BCPL string
    printf("Name = %s\n", name.name);
    name.write(p + 432);
    
    // Next block with same hash
    write32(p + 496, next);

    // Block pointer to parent directory
    write32(p + 500, parent);
    
    // Subtype
    write32(p + 508, 2);
        
    // Checksum
    write32(p + 20, FSBlock::checksum(p));
}

void
FSUserDirBlock::setNext(u32 ref)
{
    // Only proceed if a valid block number is given
    if (!volume.isBlockNumber(ref)) return;
    
    if (next) {
        volume.block(next)->setNext(ref);
    } else {
        next = ref;
    }
}

void
FSUserDirBlock::setParent(u32 ref)
{
    if (volume.isBlockNumber(ref)) parent = ref;
}

void
FSUserDirBlock::addItem(FSBlock *block)
{
    printf("addItem(%p)\n", block);

    // Only proceed if a block is given
    if (block == nullptr) return;
            
    // Add the block to the hash table
    hashTable.link(block);

    // Set the reference to the parent directory
    block->setParent(this->nr);
}
