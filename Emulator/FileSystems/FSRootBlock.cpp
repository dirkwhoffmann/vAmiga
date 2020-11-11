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
    data = new u8[ref.bsize]();

    setCreationDate(time(NULL));
    setModificationDate(time(NULL));
    
    // Type
    write32(data, 2);
    
    // Hash table size
    write32(data + 12, (volume.bsize / sizeof(u32)) - 56);
    
    // Subtype
    write32(data + volume.bsize - 4, 1);
}

FSRootBlock::FSRootBlock(FSVolume &ref, u32 nr, const char *name) : FSRootBlock(ref, nr)
{
    this->name = FSName(name);
    setName(FSName(name));
}

FSRootBlock::~FSRootBlock()
{
    delete [] data;
    // delete hashTable;
}

void
FSRootBlock::dump()
{
    printf("        Name: "); name.dump(); printf("\n");
    printf("     Created: "); dumpDate(getCreationDate()); printf("\n");
    printf("    Modified: "); dumpDate(getModificationDate()); printf("\n");
    printf("  Hash table: "); dumpHashTable(); printf("\n");
}

bool
FSRootBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    result &= checkHashTable(verbose);
    return result;
}

void
FSRootBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(p);
    assert(volume.bsize == bsize);

    memcpy(p, data, bsize);

    // Start from scratch
    // memset(p, 0, bsize);

    // Type
    // write32(p, 2);
    assert(read32(p) == 2);
    
    // Hashtable size
    // write32(p + 12, hashTable->hashTableSize);
    
    // Hashtable
    // hashTable->write(p + 24);
    
    // BM flag (true if bitmap on disk is valid)
    write32(p + bsize - 50 * 4, 0xFFFFFFFF);
    
    // BM pages (indicates the blocks containing the bitmap)
    write32(p + bsize - 49 * 4, 881);
    
    // Last recent change of the root directory of this volume
    // modified.write(p + bsize - 23 * 4);
    
    // Volume name
    name.write(p + bsize - 20 * 4);

    // Date and time when this volume was formatted
    // created.write(p + bsize - 7 * 4);
        
    // Secondary block type
    // write32(p + bsize - 1 * 4, 1);
    assert(read32(p + bsize - 1 * 4) == 1);
    
    // Compute checksum
    write32(p + 20, FSBlock::checksum(p));
}

FSName
FSRootBlock::getName()
{
    return FSName(data + bsize() - 20 * 4);
}

void
FSRootBlock::setName(FSName name)
{
    name.write(data + bsize() - 20 * 4);
}

time_t
FSRootBlock::getCreationDate()
{
    return readTimeStamp(data + bsize() - 7 * 4);
}

void
FSRootBlock::setCreationDate(time_t t)
{
    writeTimeStamp(data + bsize() - 7 * 4, t);
}

time_t
FSRootBlock::getModificationDate()
{
    return readTimeStamp(data + bsize() - 23 * 4);
}

void
FSRootBlock::setModificationDate(time_t t)
{
    writeTimeStamp(data + bsize() - 23 * 4, t);
}
