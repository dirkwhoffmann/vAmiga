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
    printf("        Name : %s\n", getName().cStr);
    printf("     Created : "); getCreationDate().print(); printf("\n");
    printf("    Modified : "); getModificationDate().print(); printf("\n");
    printf("  Hash table : "); dumpHashTable(); printf("\n");
}

FSError
FSRootBlock::check(u32 pos)
{
    // Make sure 'pos' points to the beginning of a long word
    assert(pos % 4 == 0);

    // Translate 'pos' to a long word index
    i32 word = (pos <= 24 ? (i32)pos : (i32)pos - volume.bsize) / 4;

    u32 value = get32(word);
    
    switch (word) {
        case 0:
            return value == 2 ? FS_OK : FS_BLOCK_TYPE_ID_MISMATCH;
        case 1:
        case 2:
        case 4:
            return value == 0 ? FS_OK : FS_EXPECTED_00;
        case 5:
            return value == checksum() ? FS_OK : FS_BLOCK_CHECKSUM_ERROR;
        case -4:
        case -3:
        case -2:
            return value == 0 ? FS_OK : FS_EXPECTED_00;
        case -1:
            return value == 1 ? FS_OK : FS_BLOCK_SUBTYPE_ID_MISMATCH;
        default:
            break;
    }
        
    // Check all hash table entries
    if (word >= 6 && word < 6 + (i32)hashTableSize()) {
        return checkHashTableItem(word - 6);
    }
    
    return FS_OK;
}

bool
FSRootBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    result &= checkHashTable(verbose);
    return result;
}

void
FSRootBlock::updateChecksum()
{
    set32(5, 0);
    set32(5, checksum());
}
