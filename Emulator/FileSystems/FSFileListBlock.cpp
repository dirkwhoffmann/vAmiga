// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSFileListBlock::FSFileListBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    data = new u8[ref.bsize]();

    set32(0, 16);                         // Type
    set32(1, nr);                         // Block pointer to itself
    set32(-1, (u32)-3);                   // Sub type
}

FSFileListBlock::~FSFileListBlock()
{
    delete [] data;
}

void
FSFileListBlock::dump()
{
    printf(" Block count : %d / %d\n", getNumDataBlockRefs(), getMaxDataBlockRefs());
    printf("       First : %d\n", getFirstDataBlockRef());
    printf("Header block : %d\n", getFileHeaderRef());
    printf("   Extension : %d\n", getNextListBlockRef());
    printf(" Data blocks : ");
    for (u32 i = 0; i < getNumDataBlockRefs(); i++) printf("%d ", getDataBlockRef(i));
    printf("\n");
}

FSItemType
FSFileListBlock::itemType(u32 byte)
{
    // Intercept some special locations
    if (byte == 328) return FSI_BCPL_STRING_LENGTH;
    if (byte == 432) return FSI_BCPL_STRING_LENGTH;

    // Translate 'pos' to a (signed) long word index
    i32 word = byte / 4; if (word >= 6) word -= volume.bsize / 4;

    switch (word) {
            
        case 0:   return FSI_TYPE_ID;
        case 1:   return FSI_SELF_REF;
        case 2:   return FSI_DATA_BLOCK_REF_COUNT;
        case 3:   return FSI_UNUSED;
        case 4:   return FSI_FIRST_DATA_BLOCK_REF;
        case 5:   return FSI_CHECKSUM;
        case -50:
        case -49:
        case -4:  return FSI_UNUSED;
        case -3:  return FSI_FILEHEADER_REF;
        case -2:  return FSI_EXT_BLOCK_REF;
        case -1:  return FSI_SUBTYPE_ID;
    }
    
    return word <= -51 ? FSI_DATA_BLOCK_REF : FSI_UNUSED;
}

FSError
FSFileListBlock::check(u32 byte)
{
    // Translate 'pos' to a (signed) long word index
    i32 word = byte / 4; if (word >= 6) word -= volume.bsize / 4;
    u32 value = get32(word);

    switch (word) {
            
        case 0:   EXPECT_00000010(value, byte % 4); break;
        case 1:   EXPECT_SELFREF(value); break;
        case 3:   EXPECT_00(value); break;
        case 4:   EXPECT_DATA_BLOCK_REF(value); break;
        case 5:   EXPECT_CHECKSUM(value); break;
        case -50:
        case -4:  EXPECT_00(value); break;
        case -3:  EXPECT_PARENT_DIR_REF(value); break;
        case -2:  EXPECT_FILE_LIST_BLOCK_REF(value); break;
        case -1:  EXPECT_FFFFFFFD(value, byte % 4); break;
    }
    
    // Data block references
    if (word <= -51 && value) EXPECT_DATA_BLOCK_REF(value);
    if (word == -51) {
        if (value == 0 && getNumDataBlockRefs() > 0) {
            return FS_EXPECTED_REF;
        }
        if (value != 0 && getNumDataBlockRefs() == 0) {
            return FS_EXPECTED_NO_REF;
        }
    }
    
    return FS_OK;
}

bool
FSFileListBlock::addDataBlockRef(u32 first, u32 ref)
{
    // The caller has to ensure that this block contains free slots
    if (getNumDataBlockRefs() < getMaxDataBlockRefs()) {

        setFirstDataBlockRef(first);
        setDataBlockRef(getNumDataBlockRefs(), ref);
        incNumDataBlockRefs();
        return true;
    }

    return false;
}
