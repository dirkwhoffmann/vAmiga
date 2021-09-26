// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSBlock.h"
#include "FSBitmapBlock.h"
#include "FSBitmapExtBlock.h"
#include "FSBootBlock.h"
#include "FSDevice.h"
#include "FSDataBlock.h"
#include "FSEmptyBlock.h"
#include "FSFileHeaderBlock.h"
#include "FSFileListBlock.h"
#include "FSPartition.h"
#include "FSRootBlock.h"
#include "FSUserDirBlock.h"
#include "MemUtils.h"

FSBlock::FSBlock(FSPartition &p, Block nr, FSBlockType t) : partition(p)
{
    this->nr = nr;
    this->type = t;
}

FSBlock *
FSBlock::make(FSPartition &p, Block nr, FSBlockType type)
{
    switch (type) {

        case FS_EMPTY_BLOCK:      return new FSEmptyBlock(p, nr, type);
        case FS_BOOT_BLOCK:       return new FSBootBlock(p, nr, type);
        case FS_ROOT_BLOCK:       return new FSRootBlock(p, nr, type);
        case FS_BITMAP_BLOCK:     return new FSBitmapBlock(p, nr, type);
        case FS_BITMAP_EXT_BLOCK: return new FSBitmapExtBlock(p, nr, type);
        case FS_USERDIR_BLOCK:    return new FSUserDirBlock(p, nr, type);
        case FS_FILEHEADER_BLOCK: return new FSFileHeaderBlock(p, nr, type);
        case FS_FILELIST_BLOCK:   return new FSFileListBlock(p, nr, type);
        case FS_DATA_BLOCK_OFS:   return new OFSDataBlock(p, nr, type);
        case FS_DATA_BLOCK_FFS:   return new FFSDataBlock(p, nr, type);
            
        default:
            throw VAError(ERROR_FS_INVALID_BLOCK_TYPE);
    }
}

const char *
FSBlock::getDescription() const
{
    switch (type) {
            
        case FS_UNKNOWN_BLOCK:     return "FSBlock (Unknown)";
        case FS_EMPTY_BLOCK:       return "FSBlock (Empty)";
        case FS_BOOT_BLOCK:        return "FSBlock (Boot)";
        case FS_ROOT_BLOCK:        return "FSBlock (Root)";
        case FS_BITMAP_BLOCK:      return "FSBlock (Bitmap)";
        case FS_BITMAP_EXT_BLOCK:  return "FSBlock (ExtBitmap)";
        case FS_USERDIR_BLOCK:     return "FSBlock (UserDir)";
        case FS_FILEHEADER_BLOCK:  return "FSBlock (FileHeader)";
        case FS_FILELIST_BLOCK:    return "FSBlock (FileList)";
        case FS_DATA_BLOCK_OFS:    return "FSBlock (OFS)";
        case FS_DATA_BLOCK_FFS:    return "FSBlock (FFF)";
            
        default:
            fatalError;
    }
}

isize
FSBlock::bsize() const
{
    return partition.dev.bsize;
}

isize
FSBlock::dsize() const
{
    switch (type) {
            
        case FS_DATA_BLOCK_OFS: return bsize() - 24;
        case FS_DATA_BLOCK_FFS: return bsize();
                        
        default:
            fatalError;
    }
}

FSItemType
FSBlock::itemType(isize byte) const
{
    // Translate the byte index to a (signed) long word index
    isize word = byte / 4; if (word >= 6) word -= bsize() / 4;

    switch (type) {
            
        case FS_EMPTY_BLOCK:
            
            return FSI_UNUSED;
            
        case FS_BOOT_BLOCK:
            
            if (nr == partition.firstBlock) {
                
                if (byte <= 2) return FSI_DOS_HEADER;
                if (byte == 3) return FSI_DOS_VERSION;
                if (byte <= 7) return FSI_CHECKSUM;
            }
            
            return FSI_BOOTCODE;

        case FS_ROOT_BLOCK:
            
            if (byte == 432) return FSI_BCPL_STRING_LENGTH;
            
            switch (word) {
                case 0:   return FSI_TYPE_ID;
                case 1:
                case 2:   return FSI_UNUSED;
                case 3:   return FSI_HASHTABLE_SIZE;
                case 4:   return FSI_UNUSED;
                case 5:   return FSI_CHECKSUM;
                case -50: return FSI_BITMAP_VALIDITY;
                case -24: return FSI_BITMAP_EXT_BLOCK_REF;
                case -23: return FSI_MODIFIED_DAY;
                case -22: return FSI_MODIFIED_MIN;
                case -21: return FSI_MODIFIED_TICKS;
                case -7:  return FSI_CREATED_DAY;
                case -6:  return FSI_CREATED_MIN;
                case -5:  return FSI_CREATED_TICKS;
                case -4:
                case -3:
                case -2:  return FSI_UNUSED;
                case -1:  return FSI_SUBTYPE_ID;
                    
                default:
                    
                    if (word <= -51)                return FSI_HASH_REF;
                    if (word <= -25)                return FSI_BITMAP_BLOCK_REF;
                    if (word >= -20 && word <= -8)  return FSI_BCPL_DISK_NAME;
            }
            
            fatalError;
            
        case FS_BITMAP_BLOCK:
            
            return byte < 4 ? FSI_CHECKSUM : FSI_BITMAP;
            
        case FS_BITMAP_EXT_BLOCK:
            
            return byte < (bsize() - 4) ? FSI_BITMAP : FSI_BITMAP_EXT_BLOCK_REF;
            
        case FS_USERDIR_BLOCK:
            
            if (byte == 328) return FSI_BCPL_STRING_LENGTH;
            if (byte == 432) return FSI_BCPL_STRING_LENGTH;

            switch (word) {
                case 0:   return FSI_TYPE_ID;
                case 1:   return FSI_SELF_REF;
                case 2:
                case 3:
                case 4:   return FSI_UNUSED;
                case 5:   return FSI_CHECKSUM;
                case -50:
                case -49: return FSI_UNUSED;
                case -48: return FSI_PROT_BITS;
                case -47: return FSI_UNUSED;
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

            fatalError;
            
        case FS_FILEHEADER_BLOCK:
            
            if (byte == 328) return FSI_BCPL_STRING_LENGTH;
            if (byte == 432) return FSI_BCPL_STRING_LENGTH;

            switch (word) {
                case 0:   return FSI_TYPE_ID;
                case 1:   return FSI_SELF_REF;
                case 2:   return FSI_DATA_BLOCK_REF_COUNT;
                case 3:   return FSI_UNUSED;
                case 4:   return FSI_FIRST_DATA_BLOCK_REF;
                case 5:   return FSI_CHECKSUM;
                case -50:
                case -49: return FSI_UNUSED;
                case -48: return FSI_PROT_BITS;
                case -47: return FSI_FILESIZE;
                case -23: return FSI_CREATED_DAY;
                case -22: return FSI_CREATED_MIN;
                case -21: return FSI_CREATED_TICKS;
                case -4:  return FSI_NEXT_HASH_REF;
                case -3:  return FSI_PARENT_DIR_REF;
                case -2:  return FSI_EXT_BLOCK_REF;
                case -1:  return FSI_SUBTYPE_ID;
            }
            
            if (word <= -51)                return FSI_DATA_BLOCK_REF;
            if (word >= -46 && word <= -24) return FSI_BCPL_COMMENT;
            if (word >= -20 && word <= -5)  return FSI_BCPL_FILE_NAME;

            fatalError;
            
        case FS_FILELIST_BLOCK:
            
            if (byte == 328) return FSI_BCPL_STRING_LENGTH;
            if (byte == 432) return FSI_BCPL_STRING_LENGTH;

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
            
        case FS_DATA_BLOCK_OFS:
                        
            switch (word) {
                    
                case 0: return FSI_TYPE_ID;
                case 1: return FSI_FILEHEADER_REF;
                case 2: return FSI_DATA_BLOCK_NUMBER;
                case 3: return FSI_DATA_COUNT;
                case 4: return FSI_NEXT_DATA_BLOCK_REF;
                case 5: return FSI_CHECKSUM;
            }
            
            return FSI_DATA;
        
        case FS_DATA_BLOCK_FFS:
            
            return FSI_DATA;
            
        default:
            fatalError;
    }
}

u32
FSBlock::typeID() const
{
    return get32(0);
}

u32
FSBlock::subtypeID() const
{
    return get32((bsize() / 4) - 1);
}

isize
FSBlock::check(bool strict) const
{
    ErrorCode error;
    isize count = 0;
    u8 expected;
    
    for (isize i = 0; i < bsize(); i++) {
        
        if ((error = check(i, &expected, strict)) != ERROR_OK) {
            count++;
            debug(FS_DEBUG, "Block %d [%zd.%zd]: %s\n", nr, i / 4, i % 4,
                  ErrorCodeEnum::key(error));
        }
    }
    
    return count;
}

ErrorCode
FSBlock::check(isize byte, u8 *expected, bool strict) const
{
    switch (type) {
            
        case FS_BOOT_BLOCK:
        {
            isize word = byte / 4;
            u32 value = data[byte];

            if (nr == partition.firstBlock) {
                         
                if (byte == 0) EXPECT_BYTE('D');
                if (byte == 1) EXPECT_BYTE('O');
                if (byte == 2) EXPECT_BYTE('S');
                if (byte == 3) EXPECT_DOS_REVISION;
                if (word == 1) { value = get32(1); EXPECT_CHECKSUM; }
            }
            break;
        }
        case FS_ROOT_BLOCK:
        {
            isize word = byte / 4; if (word >= 6) word -= bsize() / 4;
            u32 value = get32(word);

            switch (word) {
                    
                case 0:   EXPECT_LONGWORD(2);                break;
                case 1:
                case 2:   if (strict) EXPECT_LONGWORD(0);    break;
                case 3:   if (strict) EXPECT_HASHTABLE_SIZE; break;
                case 4:   EXPECT_LONGWORD(0);                break;
                case 5:   EXPECT_CHECKSUM;                   break;
                case -50:                                    break;
                case -49: EXPECT_BITMAP_REF;                 break;
                case -24: EXPECT_OPTIONAL_BITMAP_EXT_REF;    break;
                case -4:
                case -3:
                case -2:  if (strict) EXPECT_LONGWORD(0);    break;
                case -1:  EXPECT_LONGWORD(1);                break;
                    
                default:
                    
                    // Hash table area
                    if (word <= -51) { EXPECT_OPTIONAL_HASH_REF; break; }
                    
                    // Bitmap block area
                    if (word <= -25) { EXPECT_OPTIONAL_BITMAP_REF; break; }
            }
            break;
        }
        case FS_BITMAP_BLOCK:
        {
            isize word = byte / 4;
            u32 value = get32(word);
            
            if (word == 0) EXPECT_CHECKSUM;
            break;
        }
        case FS_BITMAP_EXT_BLOCK:
        {
            isize word = byte / 4;
            u32 value = get32(word);
            
            if (word == (i32)(bsize() - 4)) EXPECT_OPTIONAL_BITMAP_EXT_REF;
            break;
        }
        case FS_USERDIR_BLOCK:
        {
            isize word = byte / 4; if (word >= 6) word -= bsize() / 4;
            u32 value = get32(word);
            
            switch (word) {
                case  0: EXPECT_LONGWORD(2);        break;
                case  1: EXPECT_SELFREF;            break;
                case  2:
                case  3:
                case  4: EXPECT_BYTE(0);            break;
                case  5: EXPECT_CHECKSUM;           break;
                case -4: EXPECT_OPTIONAL_HASH_REF;  break;
                case -3: EXPECT_PARENT_DIR_REF;     break;
                case -2: EXPECT_BYTE(0);            break;
                case -1: EXPECT_LONGWORD(2);        break;
            }
            if (word <= -51) EXPECT_OPTIONAL_HASH_REF;
            break;
        }
        case FS_FILEHEADER_BLOCK:
        {
            /* Note: At locations -4 and -3, many disks reference the bitmap
             * block which is wrong. We ignore to report this common
             * inconsistency if 'strict' is set to false.
             */

            // Translate the byte index to a (signed) long word index
            isize word = byte / 4; if (word >= 6) word -= bsize() / 4;
            u32 value = get32(word);
            
            switch (word) {
                case   0: EXPECT_LONGWORD(2);                    break;
                case   1: EXPECT_SELFREF;                        break;
                case   3: EXPECT_BYTE(0);                        break;
                case   4: EXPECT_DATABLOCK_REF;                  break;
                case   5: EXPECT_CHECKSUM;                       break;
                case -50: EXPECT_BYTE(0);                        break;
                case  -4: if (strict) EXPECT_OPTIONAL_HASH_REF;  break;
                case  -3: if (strict) EXPECT_PARENT_DIR_REF;     break;
                case  -2: EXPECT_OPTIONAL_FILELIST_REF;          break;
                case  -1: EXPECT_LONGWORD(-3);                   break;
            }
                
            // Data block reference area
            if (word <= -51 && value) EXPECT_DATABLOCK_REF;
            if (word == -51) {
                if (value == 0 && getNumDataBlockRefs() > 0) {
                    return ERROR_FS_EXPECTED_REF;
                }
                if (value != 0 && getNumDataBlockRefs() == 0) {
                    return ERROR_FS_EXPECTED_NO_REF;
                }
            }
            break;
        }
        case FS_FILELIST_BLOCK:
        {
            /* Note: At location -3, many disks reference the bitmap
             * block which is wrong. We ignore to report this common
             * inconsistency if 'strict' is set to false.
             */
            
            // Translate 'pos' to a (signed) long word index
            isize word = byte / 4; if (word >= 6) word -= bsize() / 4;
            u32 value = get32(word);

            switch (word) {
                    
                case   0: EXPECT_LONGWORD(16);                break;
                case   1: EXPECT_SELFREF;                     break;
                case   3: EXPECT_BYTE(0);                     break;
                case   4: EXPECT_OPTIONAL_DATABLOCK_REF;      break;
                case   5: EXPECT_CHECKSUM;                    break;
                case -50:
                case  -4: EXPECT_BYTE(0);                     break;
                case  -3: if (strict) EXPECT_FILEHEADER_REF;  break;
                case  -2: EXPECT_OPTIONAL_FILELIST_REF;       break;
                case  -1: EXPECT_LONGWORD(-3);                break;
            }
            
            // Data block references
            if (word <= -51 && value) EXPECT_DATABLOCK_REF;
            if (word == -51) {
                if (value == 0 && getNumDataBlockRefs() > 0) {
                    return ERROR_FS_EXPECTED_REF;
                }
                if (value != 0 && getNumDataBlockRefs() == 0) {
                    return ERROR_FS_EXPECTED_NO_REF;
                }
            }
            break;
        }
                  
        case FS_DATA_BLOCK_OFS:
        {
            /* Note: At location 1, many disks store a reference to the bitmap
             * block instead of a reference to the file header block. We ignore
             * to report this common inconsistency if 'strict' is set to false.
             */

            if (byte < 24) {
                
                isize word = byte / 4;
                u32 value = get32(word);
                        
                switch (word) {
                        
                    case 0: EXPECT_LONGWORD(8);                 break;
                    case 1: if (strict) EXPECT_FILEHEADER_REF;  break;
                    case 2: EXPECT_DATABLOCK_NUMBER;            break;
                    case 3: EXPECT_LESS_OR_EQUAL(dsize());      break;
                    case 4: EXPECT_OPTIONAL_DATABLOCK_REF;      break;
                    case 5: EXPECT_CHECKSUM;                    break;
                }
            }
            break;
        }

        default:
            break;
    }
    
    return ERROR_OK;
}

u8 *
FSBlock::addr32(isize nr) const
{
    return (data + 4 * nr) + (nr < 0 ? bsize() : 0);
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

isize
FSBlock::checksumLocation() const
{
    switch (type) {
            
        case FS_BOOT_BLOCK:

            return (nr == partition.firstBlock) ? 1 : -1;
            
        case FS_BITMAP_BLOCK:
            
            return 0;
            
        case FS_ROOT_BLOCK:
        case FS_USERDIR_BLOCK:
        case FS_FILEHEADER_BLOCK:
        case FS_FILELIST_BLOCK:
        case FS_DATA_BLOCK_OFS:
            
            return 5;
            
        default:
            
            return -1;
    }
}

u32
FSBlock::checksum() const
{
    return type == FS_BOOT_BLOCK ? checksumBootBlock() : checksumStandard();
}

u32
FSBlock::checksumStandard() const
{
    isize pos = checksumLocation();
    assert(pos >= 0 && pos <= 5);
    
    // Wipe out the old checksum
    u32 old = get32(pos);
    set32(pos, 0);
    
    // Compute the new checksum
    u32 result = 0;
    for (isize i = 0; i < bsize() / 4; i++) result += get32(i);
    result = ~result + 1;
    
    // Undo the modification
    set32(pos, old);
    
    return result;
}

u32
FSBlock::checksumBootBlock() const
{
    // Only call this function for the first boot block in a partition
    assert(nr == partition.firstBlock);
        
    u32 result = get32(0), prec;

    // First boot block
    for (isize i = 2; i < bsize() / 4; i++) {
        
        prec = result;
        if ( (result += get32(i)) < prec) result++;
    }

    // Second boot block
    u8 *p = partition.dev.blocks[1]->data;
    
    for (isize i = 0; i < bsize() / 4; i++) {
        
        prec = result;
        if ( (result += FSBlock::read32(p + 4*i)) < prec) result++;
    }

    return ~result;
}

void
FSBlock::updateChecksum()
{
    isize pos = checksumLocation();
    if (pos >= 0 && pos < bsize() / 4) set32(pos, checksum());
}

void
FSBlock::dump() const
{
    switch (type) {
                        
        case FS_BOOT_BLOCK:
            
            msg("       Header : ");
            for (isize i = 0; i < 8; i++) msg("%02X ", data[i]);
            msg("\n");
            break;
            
        case FS_ROOT_BLOCK:
        {
            msg("         Name : %s\n", getName().c_str());
            msg("      Created : %s\n", getCreationDate().str().c_str());
            msg("     Modified : %s\n", getModificationDate().str().c_str());
            msg("   Hash table : "); dumpHashTable(); printf("\n");
            msg("Bitmap blocks : ");
            for (isize i = 0; i < 25; i++) {
                if (isize ref = getBmBlockRef(i)) msg("%zd ", ref);
            }
            msg("\n");
            msg("   Next BmExt : %d\n", getNextBmExtBlockRef());
            break;
        }
        case FS_BITMAP_BLOCK:
        {
            isize count = 0;
            for (isize i = 1; i < bsize() / 4; i++) {
                if (u32 value = get32(i)) {
                    for (isize j = 0; j < 32; j++) {
                        if (GET_BIT(value, j)) count++;
                    }
                }
            }
            msg("         Free : %zd blocks\n", count);
        }
        case FS_BITMAP_EXT_BLOCK:
        {
            msg("Bitmap blocks : ");
            for (isize i = 0; i < (bsize() / 4) - 1; i++) {
                if (Block ref = getBmBlockRef(i)) msg("%d ", ref);
            }
            msg("\n");
            msg("         Next : %d\n", getNextBmExtBlockRef());
            break;
        }
        case FS_USERDIR_BLOCK:
            
            printf("        Name: %s\n", getName().c_str());
            printf("     Comment: %s\n", getComment().c_str());
            printf("     Created: %s\n", getCreationDate().str().c_str());
            printf("      Parent: %d\n", getParentDirRef());
            printf("        Next: %d\n", getNextHashRef());
            break;
            
        case FS_FILEHEADER_BLOCK:
            
            msg("           Name : %s\n", getName().c_str());
            msg("        Comment : %s\n", getComment().c_str());
            msg("        Created : %s\n", getCreationDate().str().c_str());
            msg("           Next : %d\n", getNextHashRef());
            msg("      File size : %d\n", getFileSize());

            msg("    Block count : %zd / %zd\n", getNumDataBlockRefs(), getMaxDataBlockRefs());
            msg("          First : %d\n", getFirstDataBlockRef());
            msg("     Parent dir : %d\n", getParentDirRef());
            msg(" FileList block : %d\n", getNextListBlockRef());
            
            msg("    Data blocks : ");
            for (isize i = 0; i < getNumDataBlockRefs(); i++) msg("%d ", getDataBlockRef(i));
            msg("\n");
            break;
            
        case FS_FILELIST_BLOCK:
            
            msg(" Block count : %zd / %zd\n", getNumDataBlockRefs(), getMaxDataBlockRefs());
            msg("       First : %d\n", getFirstDataBlockRef());
            msg("Header block : %d\n", getFileHeaderRef());
            msg("   Extension : %d\n", getNextListBlockRef());
            msg(" Data blocks : ");
            for (isize i = 0; i < getNumDataBlockRefs(); i++) msg("%d ", getDataBlockRef(i));
            msg("\n");
            break;
            
        case FS_DATA_BLOCK_OFS:
            
            msg("File header block : %d\n", getFileHeaderRef());
            msg("     Chain number : %d\n", getDataBlockNr());
            msg("       Data bytes : %d\n", getDataBytesInBlock());
            msg("  Next data block : %d\n", getNextDataBlockRef());
            msg("\n");
            break;
            
        default:
            break;
    }
}

void
FSBlock::dumpData() const
{
    if (data) util::hexdumpLongwords(data, 512);
}

void
FSBlock::importBlock(const u8 *src, isize size)
{    
    assert(size == bsize());
    assert(src != nullptr);
    assert(data != nullptr);
        
    std::memcpy(data, src, size);
}

void
FSBlock::exportBlock(u8 *dst, isize size)
{
    assert(size == bsize());
            
    // Rectify the checksum
    updateChecksum();

    // Export the block
    assert(dst != nullptr);
    assert(data != nullptr);
    std::memcpy(dst, data, size);
}

FSBlock *
FSBlock::getParentDirBlock()
{
    Block nr = getParentDirRef();
    return nr ? partition.dev.blockPtr(nr) : nullptr;
}

FSFileHeaderBlock *
FSBlock::getFileHeaderBlock()
{
    Block nr = getFileHeaderRef();
    return nr ? partition.dev.fileHeaderBlockPtr(nr) : nullptr;
}

FSBlock *
FSBlock::getNextHashBlock()
{
    Block nr = getNextHashRef();
    return nr ? partition.dev.blockPtr(nr) : nullptr;
}

FSFileListBlock *
FSBlock::getNextListBlock()
{
    Block nr = getNextListBlockRef();
    return nr ? partition.dev.fileListBlockPtr(nr) : nullptr;
}

FSBitmapExtBlock *
FSBlock::getNextBmExtBlock()
{
    Block nr = getNextBmExtBlockRef();
    return nr ? partition.dev.bitmapExtBlockPtr(nr) : nullptr;
}


FSDataBlock *
FSBlock::getFirstDataBlock()
{
    Block nr = getFirstDataBlockRef();
    return nr ? partition.dev.dataBlockPtr(nr) : nullptr;
}

Block
FSBlock::getDataBlockRef(isize nr) const
{
    switch (type) {
            
        case FS_FILEHEADER_BLOCK:
        case FS_FILELIST_BLOCK:
            
            return get32(-51 - nr);
            
        default:
            fatalError;
    }
}

void
FSBlock::setDataBlockRef(isize nr, Block ref)
{
    switch (type) {
            
        case FS_FILEHEADER_BLOCK:
        case FS_FILELIST_BLOCK:
            
            set32(-51-nr, ref);
            return;
            
        default:
            fatalError;
    }
}

FSDataBlock *
FSBlock::getNextDataBlock()
{
    Block nr = getNextDataBlockRef();
    return nr ? partition.dev.dataBlockPtr(nr) : nullptr;
}

u32
FSBlock::getHashRef(Block nr) const
{
    return (nr < (Block)hashTableSize()) ? get32(6 + nr) : 0;
}

void
FSBlock::setHashRef(Block nr, u32 ref)
{
    if (nr < (Block)hashTableSize()) set32(6 + nr, ref);
}

void
FSBlock::dumpHashTable() const
{
    for (isize i = 0; i < hashTableSize(); i++) {
        
        u32 value = read32(data + 24 + 4 * i);
        if (value) {
            msg("%zd: %d ", i, value);
        }
    }
}

Block
FSBlock::getBmBlockRef(isize nr) const
{
    switch (type) {
            
        case FS_ROOT_BLOCK:
            
            return get32(nr - 49);
            
        case FS_BITMAP_EXT_BLOCK:
            
            return get32(nr);
            
        default:
            fatalError;
    }
}

void
FSBlock::setBmBlockRef(isize nr, Block ref)
{
    switch (type) {
            
        case FS_ROOT_BLOCK:
            
            set32(nr - 49, ref);
            return;
            
        case FS_BITMAP_EXT_BLOCK:
            
            set32(nr, ref);
            return;
            
        default:
            fatalError;
    }
}

u32
FSBlock::getDataBlockNr() const
{
    switch (type) {
            
        case FS_DATA_BLOCK_OFS: return get32(2); 
        case FS_DATA_BLOCK_FFS: return 0;
                        
        default:
            fatalError;
    }
}

void
FSBlock::setDataBlockNr(u32 val)
{
    switch (type) {
            
        case FS_DATA_BLOCK_OFS: set32(2, val); break;
        case FS_DATA_BLOCK_FFS: break;
                        
        default:
            fatalError;
    }
}

isize
FSBlock::getMaxDataBlockRefs() const
{
    return bsize() / 4 - 56;
}

u32
FSBlock::getDataBytesInBlock() const
{
    switch (type) {
            
        case FS_DATA_BLOCK_OFS: return get32(3);
        case FS_DATA_BLOCK_FFS: return 0;
                        
        default:
            fatalError;
    }
}

void
FSBlock::setDataBytesInBlock(u32 val)
{
    switch (type) {
            
        case FS_DATA_BLOCK_OFS: set32(3, val); break;
        case FS_DATA_BLOCK_FFS: break;
                        
        default:
            fatalError;
    }
}
