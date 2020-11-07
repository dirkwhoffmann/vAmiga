// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSRootBlock::FSRootBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    hashTable = new FSHashTable(volume);
}

FSRootBlock::FSRootBlock(FSVolume &ref, u32 nr, const char *name) : FSRootBlock(ref, nr)
{
    this->name = FSName(name);
}

FSRootBlock::~FSRootBlock()
{
    delete hashTable;
}

void
FSRootBlock::dump()
{
    printf("        Name: "); name.dump(); printf("\n");
    printf("     Created: "); created.dump(); printf("\n");
    printf("    Modified: "); modified.dump(); printf("\n");
    printf("  Hash table: "); hashTable->dump(); printf("\n");
}

bool
FSRootBlock::check(bool verbose)
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
FSRootBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(p);
    assert(volume.bsize == bsize);

    // Start from scratch
    memset(p, 0, bsize);

    // Type
    write32(p, 2);
    
    // Hashtable size
    write32(p + 12, hashTable->hashTableSize);
    
    // Hashtable
    hashTable->write(p + 24);
    
    // BM flag (true if bitmap on disk is valid)
    write32(p + bsize - 50 * 4, 0xFFFFFFFF);
    
    // BM pages (indicates the blocks containing the bitmap)
    write32(p + bsize - 49 * 4, 881);
    
    // Last recent change of the root directory of this volume
    modified.write(p + bsize - 23 * 4);
    
    // Volume name
    name.write(p + bsize - 20 * 4);

    // Date and time when this volume was formatted
    created.write(p + bsize - 7 * 4);
        
    // Secondary block type
    write32(p + bsize - 1 * 4, 1);
    
    // Compute checksum
    write32(p + 20, FSBlock::checksum(p));
}
