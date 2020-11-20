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

FSItemType
FSUserDirBlock::itemType(u32 byte)
{
    // Intercept some special locations
    if (byte == 328) return FSI_BCPL_STRING_LENGTH;
    if (byte == 432) return FSI_BCPL_STRING_LENGTH;

    // Translate the byte index to a (signed) long word index
    i32 word = byte / 4; if (word >= 6) word -= volume.bsize / 4;

    switch (word) {
        case 0:   return FSI_TYPE_ID;
        case 1:   return FSI_SELF_REF;
        case 2:
        case 3:
        case 4:   return FSI_UNUSED;
        case 5:   return FSI_CHECKSUM;
        case -50:
        case -49: return FSI_UNUSED;
        case -23: return FSI_CREATED_DAY;
        case -22: return FSI_CREATED_MIN;
        case -21: return FSI_CREATED_TICKS;
        case -4:  return FSI_NEXT_HASH_REF;
        case -3:  return FSI_PARENT_DIR_REF;
        case -2:  return FSI_UNUSED;
        case -1:  return FSI_SUBTYPE_ID;
    }
    
    if (word <= -51)                return FSI_HASH_REF;
    if (word >= -46 && word <= -24) return FSI_BCPL_COMMENT;
    if (word >= -20 && word <= -5)  return FSI_BCPL_DIR_NAME;

    assert(false);
    return FSI_UNKNOWN;
}

FSError
FSUserDirBlock::check(u32 byte)
{
    // Translate the byte index to a (signed) long word index
    i32 word = byte / 4; if (word >= 6) word -= volume.bsize / 4;
    u32 value = get32(word);
    
    switch (word) {
        case 0:  EXPECT_02(value); break;
        case 1:  EXPECT_SELFREF(value); break;
        case 2:
        case 3:
        case 4:  EXPECT_00(value); break;
        case 5:  EXPECT_CHECKSUM(value); break;
        case -4: EXPECT_REF(value); break;
        case -3: EXPECT_PARENT_DIR_REF(value); break;
        case -2: EXPECT_00(value); break;
        case -1: EXPECT_02(value); break;
    }
    if (word <= -51) EXPECT_HASH_REF(value);
    
    return FS_OK;
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

void
FSUserDirBlock::updateChecksum()
{
    set32(5, 0);
    set32(5, checksum());
}
