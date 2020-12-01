// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Utils.h"
#include "FSDevice.h"

FSBlock *
FSBlock::makeWithType(FSDevice &ref, u32 nr, FSBlockType type, FSVolumeType dos)
{
    switch (type) {

        case FS_EMPTY_BLOCK:      return new FSEmptyBlock(ref, nr);
        case FS_BOOT_BLOCK:       return new FSBootBlock(ref, nr, dos);
        case FS_ROOT_BLOCK:       return new FSRootBlock(ref, nr);
        case FS_BITMAP_BLOCK:     return new FSBitmapBlock(ref, nr);
        case FS_BITMAP_EXT_BLOCK: return new FSBitmapExtBlock(ref, nr);
        case FS_USERDIR_BLOCK:    return new FSUserDirBlock(ref, nr);
        case FS_FILEHEADER_BLOCK: return new FSFileHeaderBlock(ref, nr);
        case FS_FILELIST_BLOCK:   return new FSFileListBlock(ref, nr);
        case FS_DATA_BLOCK_OFS:   return new OFSDataBlock(ref, nr);
        case FS_DATA_BLOCK_FFS:   return new FFSDataBlock(ref, nr);
            
        default:                  return nullptr;
    }
}

u32
FSBlock::typeID()
{
    return get32(0);
}

u32
FSBlock::subtypeID()
{
    return get32((volume.bsize / 4) - 1);
}

unsigned
FSBlock::check(bool strict)
{
    FSError error;
    unsigned count = 0;
    u8 expected;
    
    for (u32 i = 0; i < volume.bsize; i++) {

        if ((error = check(i, &expected, strict)) != FS_OK) {
            count++;
             debug(FS_DEBUG, "Block %d [%d.%d]: %s\n", nr, i / 4, i % 4, sFSError(error));
        }
    }

    return count;
}

u8 *
FSBlock::addr32(int nr)
{
    return (data + 4 * nr) + (nr < 0 ? volume.bsize : 0);
}

u32
FSBlock::read32(const u8 *p)
{
    return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
}
    
void
FSBlock::write32(u8 *p, u32 value)
{
    p[0] = (value >> 24) & 0xFF;
    p[1] = (value >> 16) & 0xFF;
    p[2] = (value >>  8) & 0xFF;
    p[3] = (value >>  0) & 0xFF;
}

void
FSBlock::dumpData()
{
    hexdumpLongwords(data, 512);
}

u32
FSBlock::checksum()
{
    u32 loc = checksumLocation();
    assert(loc <= 5);
    
    // Wipe out the old checksum
    u32 old = get32(loc);
    set32(loc, 0);
    
    // Compute the new checksum
    u32 result = 0;
    for (u32 i = 0; i < volume.bsize / 4; i++) result += get32(i);
    result = ~result + 1;
    
    // Undo the modification
    set32(loc, old);
    
    return result;
}

void
FSBlock::updateChecksum()
{
    u32 ref = checksumLocation();
    if (ref < volume.bsize / 4) set32(ref, checksum());
}

void
FSBlock::importBlock(const u8 *src, size_t bsize)
{    
    assert(bsize == volume.bsize);
    assert(src != nullptr);
    assert(data != nullptr);
        
    memcpy(data, src, bsize);
}

void
FSBlock::exportBlock(u8 *dst, size_t bsize)
{
    assert(bsize == volume.bsize);
            
    // Rectify the checksum
    updateChecksum();

    // Export the block
    assert(dst != nullptr);
    assert(data != nullptr);
    memcpy(dst, data, bsize);
}

FSBlock *
FSBlock::getParentDirBlock()
{
    u32 ref = getParentDirRef();
    return ref ? volume.block(ref) : nullptr;
}

FSFileHeaderBlock *
FSBlock::getFileHeaderBlock()
{
    u32 ref = getFileHeaderRef();
    return ref ? volume.fileHeaderBlock(ref) : nullptr;
}

FSBlock *
FSBlock::getNextHashBlock()
{
    u32 ref = getNextHashRef();
    return ref ? volume.block(ref) : nullptr;
}

FSFileListBlock *
FSBlock::getNextListBlock()
{
    u32 ref = getNextListBlockRef();
    return ref ? volume.fileListBlock(ref) : nullptr;
}

FSBitmapExtBlock *
FSBlock::getNextBmExtBlock()
{
    u32 ref = getNextBmExtBlockRef();
    return ref ? volume.bitmapExtBlock(ref) : nullptr;
}


FSDataBlock *
FSBlock::getFirstDataBlock()
{
    u32 ref = getFirstDataBlockRef();
    return ref ? volume.dataBlock(ref) : nullptr;
}

FSDataBlock *
FSBlock::getNextDataBlock()
{
    u32 ref = getNextDataBlockRef();
    return ref ? volume.dataBlock(ref) : nullptr;
}

u32
FSBlock::getHashRef(u32 nr)
{
    return (nr < hashTableSize()) ? get32(6 + nr) : 0;
}

void
FSBlock::setHashRef(u32 nr, u32 ref)
{
    if (nr < hashTableSize()) set32(6 + nr, ref);
}

void
FSBlock::dumpHashTable()
{
    for (u32 i = 0; i < hashTableSize(); i++) {
        
        u32 value = read32(data + 24 + 4 * i);
        if (value) {
            msg("%d: %d ", i, value);
        }
    }
}

u32
FSBlock::getMaxDataBlockRefs()
{
    return volume.bsize / 4 - 56;
}
