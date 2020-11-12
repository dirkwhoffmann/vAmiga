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
    
    //
    // Setup constant values
    //
    
    assert(hashTableSize() == 72);
    
    set32(0, 2);                         // Type
    set32(3, hashTableSize());           // Hash table size
    set32(-49, volume.bitmapBlockNr());  // Location of the bitmap block
    set32(-50, 0xFFFFFFFF);              // Bitmap validity
    setCreationDate(time(NULL));         // Creation date
    setModificationDate(time(NULL));     // Modification date
    set32(-1, 1);                        // Sub type
}

FSRootBlock::FSRootBlock(FSVolume &ref, u32 nr, const char *name) : FSRootBlock(ref, nr)
{
    setName(FSName(name));
}

FSRootBlock::~FSRootBlock()
{
    delete [] data;
}

void
FSRootBlock::dump()
{
    printf("        Name: %s\n", getName().cStr);
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

/*
void
FSRootBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(p);
    assert(volume.bsize == bsize);
    
    // Rectify the checksum
    updateChecksum();

    // Export the block
    memcpy(p, data, bsize);
}
*/

void
FSRootBlock::updateChecksum()
{
    set32(5, 0);
    set32(5, checksum(data));
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
