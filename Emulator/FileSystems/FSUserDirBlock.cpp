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
    data = new u8[ref.bsize]();
        
    set32(0, 2);                         // Type
    set32(1, nr);                        // Block pointer to itself
    setCreationDate(time(NULL));         // Creation date
    set32(-1, 2);                        // Sub type
}

FSUserDirBlock::FSUserDirBlock(FSVolume &ref, u32 nr, const char *name) : FSUserDirBlock(ref, nr)
{
    setName(FSName(name));
}

FSUserDirBlock::~FSUserDirBlock()
{
    delete [] data;
}

void
FSUserDirBlock::dump()
{
    printf("        Name: %s\n", getName().cStr);
    printf("        Path: ");    printPath(); printf("\n");
    printf("     Comment: %s\n", getComment().cStr);
    printf("     Created: ");    getCreationDate().print(); printf("\n");
    printf("      Parent: %d\n", getParentDirRef());
    printf("        Next: %d\n", getNextHashRef());
}

FSError
FSUserDirBlock::check(u32 pos)
{
    // Align pos to the long word raster
    pos &= ~0b11;

    // Translate 'pos' to a long word index
    i32 word = (pos <= 24 ? (i32)pos : (i32)pos - volume.bsize) / 4;

    u32 value = get32(word);
    
    switch (word) {
        case 0:
            return value == 2 ? FS_OK : FS_BLOCK_TYPE_ID_MISMATCH;
        case 1:
            return value != nr ? FS_OK : FS_BLOCK_MISSING_SELFREF;
        case 2:
        case 3:
        case 4:
            return value == 0 ? FS_OK : FS_EXPECTED_00;
        case 5:
            return value == checksum() ? FS_OK : FS_BLOCK_CHECKSUM_ERROR;
        case -4:
            return volume.block(value) ? FS_OK : FS_BLOCK_REF_OUT_OF_RANGE;
        case -3:
            if (value == 0) return FS_BLOCK_REF_MISSING;
            if (!volume.userDirBlock(value) &&
                !volume.rootBlock(value)) return FS_BLOCK_REF_TYPE_MISMATCH;
            return FS_OK;
        case -2:
            return value == 0 ? FS_OK : FS_EXPECTED_00;
        case -1:
            return value == 2 ? FS_OK : FS_BLOCK_SUBTYPE_ID_MISMATCH;
        default:
            break;
    }
        
    // Check all hash table entries
    if (word >= 6 && word < 6 + (i32)hashTableSize()) {
        return checkHashTableItem(word - 6);
    }
    
    return FS_OK;
}

void
FSUserDirBlock::updateChecksum()
{
    set32(5, 0);
    set32(5, checksum());
}
