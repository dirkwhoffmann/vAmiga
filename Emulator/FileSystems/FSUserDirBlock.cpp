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
    hashTable = new FSHashTable(volume);
}

FSUserDirBlock::FSUserDirBlock(FSVolume &ref, u32 nr, const char *name) : FSUserDirBlock(ref, nr)
{
    this->name = FSName(name);
}

FSUserDirBlock::~FSUserDirBlock()
{
    delete hashTable;
}

void
FSUserDirBlock::dump()
{
    printf("        Name: "); printName(); printf("\n");
    printf("        Path: "); printPath(); printf("\n");
    printf("     Comment: "); comment.dump(); printf("\n");
    printf("     Created: "); created.dump(); printf("\n");
    printf("  Hash table: "); hashTable->dump(); printf("\n");
    printf("      Parent: %d\n", parent);
    printf("        Next: %d\n", next);
}

bool
FSUserDirBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);

    for (int i = 0; i < hashTable->hashTableSize; i++) {
       
        u32 ref = hashTable->hashTable[i];
        if (ref == 0) continue;
        
        result &= assertInRange(ref, verbose);
        result &= assertHasType(ref, FS_USERDIR_BLOCK, FS_FILEHEADER_BLOCK, verbose);
    }

    return result;
}

void
FSUserDirBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(p);
    assert(volume.bsize == bsize);

    // Start from scratch
    memset(p, 0, bsize);
    
    // Type
    write32(p, 2);
    
    // Block pointer to itself
    write32(p + 4, nr);
    
    // Hashtable
    hashTable->write(p + 24);

    // Protection status bits
    write32(p + bsize - 48 * 4, protection);
    
    // Comment as BCPL string
    comment.write(p + bsize - 46 * 4);
    
    // Creation date
    created.write(p + bsize - 23 * 4);
    
    // Directory name as BCPL string
    name.write(p + bsize - 20 * 4);
    
    // Next block with same hash
    write32(p + bsize - 4 * 4, next);

    // Block pointer to parent directory
    write32(p + bsize - 3 * 4, parent);
    
    // Subtype
    write32(p + bsize - 1 * 4, 2);
        
    // Checksum
    write32(p + 20, FSBlock::checksum(p));
}

void
FSUserDirBlock::setNext(u32 ref)
{
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
