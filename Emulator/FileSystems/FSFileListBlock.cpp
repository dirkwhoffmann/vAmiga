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

FSError
FSFileListBlock::check(u32 pos)
{
    // Make sure 'pos' points to the beginning of a long word
    assert(pos % 4 == 0);

    // Translate 'pos' to a long word index
    i32 word = (pos <= 24 ? (i32)pos : (i32)pos - volume.bsize) / 4;

    u32 value = get32(word);
    
    switch (word) {
        case 0:
            return value == 16 ? FS_OK : FS_BLOCK_TYPE_ID_MISMATCH;
        case 1:
            return value != nr ? FS_OK : FS_BLOCK_MISSING_SELFREF;
        case 3:
            return value == 0 ? FS_OK : FS_EXPECTED_00;
        case 4:
            if (value) {
                if (!volume.block(value)) return FS_BLOCK_REF_OUT_OF_RANGE;
                if (!volume.dataBlock(value)) return FS_BLOCK_REF_TYPE_MISMATCH;
            }
            return FS_OK;
        case 5:
            return value == checksum() ? FS_OK : FS_BLOCK_CHECKSUM_ERROR;
        case -50:
        case -4:
            return value == 0 ? FS_OK : FS_EXPECTED_00;
        case -3:
            if (value == 0) return FS_BLOCK_REF_MISSING;
            if (!volume.userDirBlock(value) &&
                !volume.rootBlock(value)) return FS_BLOCK_REF_TYPE_MISMATCH;
            return FS_OK;
        case -2:
            if (value == 0) return FS_OK;
            if (!volume.fileListBlock(value)) return FS_BLOCK_REF_TYPE_MISMATCH;
            return FS_OK;
        case -1:
            return value == (u32)-3 ? FS_OK : FS_BLOCK_SUBTYPE_ID_MISMATCH;
        default:
            break;
    }
        
    // Data block reference area
    if (word >= -51 && word <= 6 && value) {
        if (!volume.dataBlock(word)) return FS_BLOCK_REF_TYPE_MISMATCH;
    }
    if (word == -51) {
        if (value == 0 && getNumDataBlockRefs() > 0) {
            return FS_BLOCK_REF_MISSING;
        }
        if (value != 0 && getNumDataBlockRefs() == 0) {
            return FS_BLOCK_UNEXPECTED_REF;
        }
    }
    
    return FS_OK;
}

void
FSFileListBlock::updateChecksum()
{
    set32(5, 0);
    set32(5, checksum());
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
