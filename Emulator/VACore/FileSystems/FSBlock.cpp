// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "FSBlock.h"
#include "MutableFileSystem.h"
#include "MemUtils.h"

namespace vamiga {

FSBlock::FSBlock(FileSystem &ref, Block nr, FSBlockType t) : device(ref)
{
    assert(t != FSBlockType::UNKNOWN_BLOCK);
    
    this->nr = nr;
    this->type = t;
    
    // Allocate memory if this block is not empty
    if (type != FSBlockType::EMPTY_BLOCK) data.init(bsize(), 0);
    
    // Initialize
    switch (type) {

        case FSBlockType::BOOT_BLOCK:
            
            if (nr == 0 && ref.dos != FSVolumeType::NODOS) {
                data[0] = 'D';
                data[1] = 'O';
                data[2] = 'S';
                data[3] = (u8)ref.dos;
            }
            break;
            
        case FSBlockType::ROOT_BLOCK:
            
            assert(hashTableSize() == 72);
            
            set32(0, 2);                         // Type
            set32(3, (u32)hashTableSize());      // Hash table size
            set32(-50, 0xFFFFFFFF);              // Bitmap validity
            setCreationDate(time(nullptr));      // Creation date
            setModificationDate(time(nullptr));  // Modification date
            set32(-1, 1);                        // Sub type
            break;
            
        case FSBlockType::USERDIR_BLOCK:

            set32(0, 2);                         // Type
            set32(1, nr);                        // Block pointer to itself
            setCreationDate(time(nullptr));      // Creation date
            set32(-1, 2);                        // Sub type
            break;
            
        case FSBlockType::FILEHEADER_BLOCK:

            set32(0, 2);                         // Type
            set32(1, nr);                        // Block pointer to itself
            setCreationDate(time(nullptr));      // Creation date
            set32(-1, (u32)-3);                  // Sub type
            break;
            
        case FSBlockType::FILELIST_BLOCK:
            
            set32(0, 16);                        // Type
            set32(1, nr);                        // Block pointer to itself
            set32(-1, (u32)-3);                  // Sub type
            break;
            
        case FSBlockType::DATA_BLOCK_OFS:
            
            set32(0, 8);                         // Block type
            break;
            
        default:
            break;
    }
}

FSBlock *
FSBlock::make(FileSystem &ref, Block nr, FSBlockType type)
{
    switch (type) {

        case FSBlockType::EMPTY_BLOCK:
        case FSBlockType::BOOT_BLOCK:
        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::BITMAP_BLOCK:
        case FSBlockType::BITMAP_EXT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
        case FSBlockType::DATA_BLOCK_OFS:
        case FSBlockType::DATA_BLOCK_FFS:
            return new FSBlock(ref, nr, type);
            
        default:
            throw CoreError(Fault::FS_INVALID_BLOCK_TYPE);
    }
}

const char *
FSBlock::objectName() const
{
    switch (type) {
            
        case FSBlockType::UNKNOWN_BLOCK:     return "FSBlock (Unknown)";
        case FSBlockType::EMPTY_BLOCK:       return "FSBlock (Empty)";
        case FSBlockType::BOOT_BLOCK:        return "FSBlock (Boot)";
        case FSBlockType::ROOT_BLOCK:        return "FSBlock (Root)";
        case FSBlockType::BITMAP_BLOCK:      return "FSBlock (Bitmap)";
        case FSBlockType::BITMAP_EXT_BLOCK:  return "FSBlock (ExtBitmap)";
        case FSBlockType::USERDIR_BLOCK:     return "FSBlock (UserDir)";
        case FSBlockType::FILEHEADER_BLOCK:  return "FSBlock (FileHeader)";
        case FSBlockType::FILELIST_BLOCK:    return "FSBlock (FileList)";
        case FSBlockType::DATA_BLOCK_OFS:    return "FSBlock (OFS)";
        case FSBlockType::DATA_BLOCK_FFS:    return "FSBlock (FFF)";
            
        default:
            fatalError;
    }
}

isize
FSBlock::bsize() const
{
    return device.bsize;
}

isize
FSBlock::dsize() const
{
    switch (type) {
            
        case FSBlockType::DATA_BLOCK_OFS: return bsize() - 24;
        case FSBlockType::DATA_BLOCK_FFS: return bsize();

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
            
        case FSBlockType::EMPTY_BLOCK:
            
            return FSItemType::UNUSED;
            
        case FSBlockType::BOOT_BLOCK:
            
            if (nr == 0) {
                
                if (byte <= 2) return FSItemType::DOS_HEADER;
                if (byte == 3) return FSItemType::DOS_VERSION;
                if (byte <= 7) return FSItemType::CHECKSUM;
            }
            
            return FSItemType::BOOTCODE;

        case FSBlockType::ROOT_BLOCK:
            
            if (byte == 432) return FSItemType::BCPL_STRING_LENGTH;
            
            switch (word) {
                case 0:   return FSItemType::TYPE_ID;
                case 1:
                case 2:   return FSItemType::UNUSED;
                case 3:   return FSItemType::HASHTABLE_SIZE;
                case 4:   return FSItemType::UNUSED;
                case 5:   return FSItemType::CHECKSUM;
                case -50: return FSItemType::BITMAP_VALIDITY;
                case -24: return FSItemType::BITMAP_EXT_BLOCK_REF;
                case -23: return FSItemType::MODIFIED_DAY;
                case -22: return FSItemType::MODIFIED_MIN;
                case -21: return FSItemType::MODIFIED_TICKS;
                case -7:  return FSItemType::CREATED_DAY;
                case -6:  return FSItemType::CREATED_MIN;
                case -5:  return FSItemType::CREATED_TICKS;
                case -4:
                case -3:
                case -2:  return FSItemType::UNUSED;
                case -1:  return FSItemType::SUBTYPE_ID;
                    
                default:
                    
                    if (word <= -51)                return FSItemType::HASH_REF;
                    if (word <= -25)                return FSItemType::BITMAP_BLOCK_REF;
                    if (word >= -20 && word <= -8)  return FSItemType::BCPL_DISK_NAME;
            }
            
            fatalError;
            
        case FSBlockType::BITMAP_BLOCK:
            
            return byte < 4 ? FSItemType::CHECKSUM : FSItemType::BITMAP;
            
        case FSBlockType::BITMAP_EXT_BLOCK:
            
            return byte < (bsize() - 4) ? FSItemType::BITMAP : FSItemType::BITMAP_EXT_BLOCK_REF;
            
        case FSBlockType::USERDIR_BLOCK:
            
            if (byte == 328) return FSItemType::BCPL_STRING_LENGTH;
            if (byte == 432) return FSItemType::BCPL_STRING_LENGTH;

            switch (word) {
                case 0:   return FSItemType::TYPE_ID;
                case 1:   return FSItemType::SELF_REF;
                case 2:
                case 3:
                case 4:   return FSItemType::UNUSED;
                case 5:   return FSItemType::CHECKSUM;
                case -50:
                case -49: return FSItemType::UNUSED;
                case -48: return FSItemType::PROT_BITS;
                case -47: return FSItemType::UNUSED;
                case -23: return FSItemType::CREATED_DAY;
                case -22: return FSItemType::CREATED_MIN;
                case -21: return FSItemType::CREATED_TICKS;
                case -4:  return FSItemType::NEXT_HASH_REF;
                case -3:  return FSItemType::PARENT_DIR_REF;
                case -2:  return FSItemType::UNUSED;
                case -1:  return FSItemType::SUBTYPE_ID;
            }
            
            if (word <= -51)                return FSItemType::HASH_REF;
            if (word >= -46 && word <= -24) return FSItemType::BCPL_COMMENT;
            if (word >= -20 && word <= -5)  return FSItemType::BCPL_DIR_NAME;

            fatalError;
            
        case FSBlockType::FILEHEADER_BLOCK:
            
            if (byte == 328) return FSItemType::BCPL_STRING_LENGTH;
            if (byte == 432) return FSItemType::BCPL_STRING_LENGTH;

            switch (word) {
                case 0:   return FSItemType::TYPE_ID;
                case 1:   return FSItemType::SELF_REF;
                case 2:   return FSItemType::DATA_BLOCK_REF_COUNT;
                case 3:   return FSItemType::UNUSED;
                case 4:   return FSItemType::FIRST_DATA_BLOCK_REF;
                case 5:   return FSItemType::CHECKSUM;
                case -50:
                case -49: return FSItemType::UNUSED;
                case -48: return FSItemType::PROT_BITS;
                case -47: return FSItemType::FILESIZE;
                case -23: return FSItemType::CREATED_DAY;
                case -22: return FSItemType::CREATED_MIN;
                case -21: return FSItemType::CREATED_TICKS;
                case -4:  return FSItemType::NEXT_HASH_REF;
                case -3:  return FSItemType::PARENT_DIR_REF;
                case -2:  return FSItemType::EXT_BLOCK_REF;
                case -1:  return FSItemType::SUBTYPE_ID;
            }
            
            if (word <= -51)                return FSItemType::DATA_BLOCK_REF;
            if (word >= -46 && word <= -24) return FSItemType::BCPL_COMMENT;
            if (word >= -20 && word <= -5)  return FSItemType::BCPL_FILE_NAME;

            fatalError;
            
        case FSBlockType::FILELIST_BLOCK:
            
            if (byte == 328) return FSItemType::BCPL_STRING_LENGTH;
            if (byte == 432) return FSItemType::BCPL_STRING_LENGTH;

            switch (word) {
                    
                case 0:   return FSItemType::TYPE_ID;
                case 1:   return FSItemType::SELF_REF;
                case 2:   return FSItemType::DATA_BLOCK_REF_COUNT;
                case 3:   return FSItemType::UNUSED;
                case 4:   return FSItemType::FIRST_DATA_BLOCK_REF;
                case 5:   return FSItemType::CHECKSUM;
                case -50:
                case -49:
                case -4:  return FSItemType::UNUSED;
                case -3:  return FSItemType::FILEHEADER_REF;
                case -2:  return FSItemType::EXT_BLOCK_REF;
                case -1:  return FSItemType::SUBTYPE_ID;
            }
            
            return word <= -51 ? FSItemType::DATA_BLOCK_REF : FSItemType::UNUSED;
            
        case FSBlockType::DATA_BLOCK_OFS:

            switch (word) {
                    
                case 0: return FSItemType::TYPE_ID;
                case 1: return FSItemType::FILEHEADER_REF;
                case 2: return FSItemType::DATA_BLOCK_NUMBER;
                case 3: return FSItemType::DATA_COUNT;
                case 4: return FSItemType::NEXT_DATA_BLOCK_REF;
                case 5: return FSItemType::CHECKSUM;
            }
            
            return FSItemType::DATA;

        case FSBlockType::DATA_BLOCK_FFS:
            
            return FSItemType::DATA;
            
        default:
            fatalError;
    }
}

u32
FSBlock::typeID() const
{
    return type == FSBlockType::EMPTY_BLOCK ? 0 : get32(0);
}

u32
FSBlock::subtypeID() const
{
    return type == FSBlockType::EMPTY_BLOCK ? 0 : get32((bsize() / 4) - 1);
}

isize
FSBlock::check(bool strict) const
{
    Fault error;
    isize count = 0;
    u8 expected;
    
    for (isize i = 0; i < bsize(); i++) {
        
        if ((error = check(i, &expected, strict)) != Fault::OK) {
            count++;
            debug(FS_DEBUG, "Block %d [%ld.%ld]: %s\n", nr, i / 4, i % 4,
                  FaultEnum::key(error));
        }
    }
    
    return count;
}

Fault
FSBlock::check(isize byte, u8 *expected, bool strict) const
{
    switch (type) {
            
        case FSBlockType::BOOT_BLOCK:
        {
            isize word = byte / 4;
            u32 value = data[byte];

            if (nr == 0) {

                if (byte == 0) EXPECT_BYTE('D');
                if (byte == 1) EXPECT_BYTE('O');
                if (byte == 2) EXPECT_BYTE('S');
                if (byte == 3) EXPECT_DOS_REVISION;
                if (word == 1) { value = get32(1); EXPECT_CHECKSUM; }
            }
            break;
        }
        case FSBlockType::ROOT_BLOCK:
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
        case FSBlockType::BITMAP_BLOCK:
        {
            isize word = byte / 4;
            u32 value = get32(word);
            
            if (word == 0) EXPECT_CHECKSUM;
            break;
        }
        case FSBlockType::BITMAP_EXT_BLOCK:
        {
            isize word = byte / 4;
            u32 value = get32(word);
            
            if (word == (i32)(bsize() - 4)) EXPECT_OPTIONAL_BITMAP_EXT_REF;
            break;
        }
        case FSBlockType::USERDIR_BLOCK:
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
        case FSBlockType::FILEHEADER_BLOCK:
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
                    return Fault::FS_EXPECTED_REF;
                }
                if (value != 0 && getNumDataBlockRefs() == 0) {
                    return Fault::FS_EXPECTED_NO_REF;
                }
            }
            break;
        }
        case FSBlockType::FILELIST_BLOCK:
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
                    return Fault::FS_EXPECTED_REF;
                }
                if (value != 0 && getNumDataBlockRefs() == 0) {
                    return Fault::FS_EXPECTED_NO_REF;
                }
            }
            break;
        }

        case FSBlockType::DATA_BLOCK_OFS:
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
    
    return Fault::OK;
}

u8 *
FSBlock::addr32(isize nr) const
{
    return (data.ptr + 4 * nr) + (nr < 0 ? bsize() : 0);
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
            
        case FSBlockType::BOOT_BLOCK:

            return nr == 0 ? 1 : -1;
            
        case FSBlockType::BITMAP_BLOCK:
            
            return 0;
            
        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
        case FSBlockType::DATA_BLOCK_OFS:
            
            return 5;
            
        default:
            
            return -1;
    }
}

u32
FSBlock::checksum() const
{
    return type == FSBlockType::BOOT_BLOCK ? checksumBootBlock() : checksumStandard();
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
    for (isize i = 0; i < bsize() / 4; i++) U32_INC(result, get32(i));
    result = ~result;
    U32_INC(result, 1);
    
    // Undo the modification
    set32(pos, old);
    
    return result;
}

u32
FSBlock::checksumBootBlock() const
{
    // Only call this function for the first boot block in a partition
    assert(nr == 0);

    u32 result = get32(0), prec;

    // First boot block
    for (isize i = 2; i < bsize() / 4; i++) {
        
        prec = result;
        if ( (result += get32(i)) < prec) result++;
    }

    // Second boot block
    u8 *p = device.blocks[1]->data.ptr;
    
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

        case FSBlockType::BOOT_BLOCK:
            
            msg("       Header : ");
            for (isize i = 0; i < 8; i++) msg("%02X ", data[i]);
            msg("\n");
            break;
            
        case FSBlockType::ROOT_BLOCK:
        {
            msg("         Name : %s\n", getName().c_str());
            msg("      Created : %s\n", getCreationDate().str().c_str());
            msg("     Modified : %s\n", getModificationDate().str().c_str());
            msg("   Hash table : "); dumpHashTable(); msg("\n");
            msg("Bitmap blocks : ");
            for (isize i = 0; i < 25; i++) {
                if (isize ref = getBmBlockRef(i)) msg("%ld ", ref);
            }
            msg("\n");
            msg("   Next BmExt : %d\n", getNextBmExtBlockRef());
            break;
        }
        case FSBlockType::BITMAP_BLOCK:
        {
            isize count = 0;
            for (isize i = 1; i < bsize() / 4; i++) {
                if (u32 value = get32(i)) {
                    for (isize j = 0; j < 32; j++) {
                        if (GET_BIT(value, j)) count++;
                    }
                }
            }
            msg("           Free : %ld blocks\n", count);
            break;
        }
        case FSBlockType::BITMAP_EXT_BLOCK:
        {
            msg("Bitmap blocks : ");
            for (isize i = 0; i < (bsize() / 4) - 1; i++) {
                if (Block ref = getBmBlockRef(i)) msg("%d ", ref);
            }
            msg("\n");
            msg("           Next : %d\n", getNextBmExtBlockRef());
            break;
        }
        case FSBlockType::USERDIR_BLOCK:
            
            msg("           Name : %s\n", getName().c_str());
            msg("        Comment : %s\n", getComment().c_str());
            msg("        Created : %s\n", getCreationDate().str().c_str());
            msg("         Parent : %d\n", getParentDirRef());
            msg("           Next : %d\n", getNextHashRef());
            break;
            
        case FSBlockType::FILEHEADER_BLOCK:
            
            msg("           Name : %s\n", getName().c_str());
            msg("        Comment : %s\n", getComment().c_str());
            msg("        Created : %s\n", getCreationDate().str().c_str());
            msg("           Next : %d\n", getNextHashRef());
            msg("      File size : %d\n", getFileSize());

            msg("    Block count : %ld / %ld\n", getNumDataBlockRefs(), getMaxDataBlockRefs());
            msg("          First : %d\n", getFirstDataBlockRef());
            msg("     Parent dir : %d\n", getParentDirRef());
            msg(" FileList block : %d\n", getNextListBlockRef());
            
            msg("    Data blocks : ");
            for (isize i = 0; i < getNumDataBlockRefs(); i++) msg("%d ", getDataBlockRef(i));
            msg("\n");
            break;
            
        case FSBlockType::FILELIST_BLOCK:
            
            msg("    Block count : %ld / %ld\n", getNumDataBlockRefs(), getMaxDataBlockRefs());
            msg("          First : %d\n", getFirstDataBlockRef());
            msg("   Header block : %d\n", getFileHeaderRef());
            msg("      Extension : %d\n", getNextListBlockRef());
            msg("    Data blocks : ");
            for (isize i = 0; i < getNumDataBlockRefs(); i++) msg("%d ", getDataBlockRef(i));
            msg("\n");
            break;
            
        case FSBlockType::DATA_BLOCK_OFS:
            
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
    if (!data.empty()) util::hexdumpLongwords(data.ptr, 512);
}

void
FSBlock::importBlock(const u8 *src, isize size)
{    
    assert(src);
    assert(size == bsize());

    if (!data.empty()) std::memcpy(data.ptr, src, size);
}

void
FSBlock::exportBlock(u8 *dst, isize size)
{
    assert(dst);
    assert(size == bsize());

    // Rectify the checksum
    updateChecksum();

    // Export the block
    if (data.empty()) {
        std::memset(dst, 0, size);
    } else {
        std::memcpy(dst, data.ptr, size);
    }
}

Fault
FSBlock::exportBlock(const fs::path &path)
{
    switch (type) {
            
        case FSBlockType::USERDIR_BLOCK:    return exportUserDirBlock(path);
        case FSBlockType::FILEHEADER_BLOCK: return exportFileHeaderBlock(path);
            
        default:
            return Fault::OK;
    }
}

Fault
FSBlock::exportUserDirBlock(const fs::path &path)
{
    auto name = path / device.getPath(this);
    
    if (!util::createDirectory(name.string())) {
        return Fault::FS_CANNOT_CREATE_DIR;
    }

    return Fault::OK;
}

Fault
FSBlock::exportFileHeaderBlock(const fs::path &path)
{
    auto filename = path / device.getPath(this);
    
    std::ofstream file(filename.string(), std::ofstream::binary);
    if (!file.is_open()) return Fault::FS_CANNOT_CREATE_FILE;
    
    writeData(file);
    return Fault::OK;
}

FSName
FSBlock::getName() const
{
    switch (type) {

        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
            
            return FSName(addr32(-20));

        default:
            return FSName("");
    }
}

void
FSBlock::setName(FSName name)
{
    switch (type) {

        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
            
            name.write(addr32(-20));

        default:
            break;
    }
}

bool
FSBlock::isNamed(FSName &other) const
{
    switch (type) {

        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
            
            return getName() == other;
            
        default:
            
            return false;
    }
}

FSComment
FSBlock::getComment() const
{
    switch (type) {

        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:

            return FSComment(addr32(-46));

        default:
            
            return FSComment("");
    }
}

void
FSBlock::setComment(FSComment name)
{
    switch (type) {

        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
            
            name.write(addr32(-46));

        default:
            
            break;
    }
}

FSTime
FSBlock::getCreationDate() const
{
    switch (type) {
            
        case FSBlockType::ROOT_BLOCK:
            
            return FSTime(addr32(-7));
            
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
            
            return FSTime(addr32(-23));
            
        default:
            return FSTime((time_t)0);
    }
}

void
FSBlock::setCreationDate(FSTime t)
{
    switch (type) {
            
        case FSBlockType::ROOT_BLOCK:

            t.write(addr32(-7));
            break;
            
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:

            t.write(addr32(-23));
            break;

        default:
            break;
    }
}

FSTime
FSBlock::getModificationDate() const
{
    switch (type) {
            
        case FSBlockType::ROOT_BLOCK:
            
            return FSTime(addr32(-23));

        default:
            return FSTime((time_t)0);
    }
}

void
FSBlock::setModificationDate(FSTime t)
{
    switch (type) {
            
        case FSBlockType::ROOT_BLOCK:
            
            t.write(addr32(-23));
            break;

        default:
            break;
    }
}

u32
FSBlock::getProtectionBits() const
{
    switch (type) {

        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:

            return get32(-48);

        default:
            return 0;
    }
}

void
FSBlock::setProtectionBits(u32 val)
{
    switch (type) {

        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:

            set32(-48, val);
            break;
            
        default:
            break;
    }
}

u32
FSBlock::getFileSize() const
{
    switch (type) {

        case FSBlockType::FILEHEADER_BLOCK:

            return get32(-47);

        default:
            return 0;
    }
}

void
FSBlock::setFileSize(u32 val)
{
    switch (type) {

        case FSBlockType::FILEHEADER_BLOCK:

            set32(-47, val);
            break;
            
        default:
            break;
    }
}

Block
FSBlock::getParentDirRef() const
{
    switch (type) {

        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:

            return get32(-3);

        default:
            return 0;
    }
}

void
FSBlock::setParentDirRef(Block ref)
{
    switch (type) {

        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:

            set32(-3, ref);
            break;
            
        default:
            break;
    }
}

FSBlock *
FSBlock::getParentDirBlock()
{
    Block nr = getParentDirRef();
    return nr ? device.blockPtr(nr) : nullptr;
}

Block
FSBlock::getFileHeaderRef() const
{
    switch (type) {
            
        case FSBlockType::FILELIST_BLOCK:  return get32(-3);
        case FSBlockType::DATA_BLOCK_OFS:  return get32(1);
            
        default:
            return 0;
    }
}

void
FSBlock::setFileHeaderRef(Block ref)
{
    switch (type) {

        case FSBlockType::FILELIST_BLOCK:  set32(-3, ref); break;
        case FSBlockType::DATA_BLOCK_OFS:  set32(1, ref); break;
            
        default:
            break;
    }
}

FSBlock *
FSBlock::getFileHeaderBlock()
{
    Block nr = getFileHeaderRef();
    return nr ? device.fileHeaderBlockPtr(nr) : nullptr;
}

Block
FSBlock::getNextHashRef() const
{
    switch (type) {

        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:

            return get32(-4);
            
        default:
            return 0;
    }
}

void
FSBlock::setNextHashRef(Block ref)
{
    switch (type) {

        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:

            set32(-4, ref);
            break;
            
        default:
            break;
    }
}

FSBlock *
FSBlock::getNextHashBlock()
{
    Block nr = getNextHashRef();
    return nr ? device.blockPtr(nr) : nullptr;
}

Block
FSBlock::getNextListBlockRef() const
{
    switch (type) {

        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
            
            return get32(-2);
            
        default:
            return 0;
    }
}

void
FSBlock::setNextListBlockRef(Block ref)
{
    switch (type) {

        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
            
            set32(-2, ref);
            break;
            
        default:
            break;
    }
}

FSBlock *
FSBlock::getNextListBlock()
{
    Block nr = getNextListBlockRef();
    return nr ? device.fileListBlockPtr(nr) : nullptr;
}

Block
FSBlock::getNextBmExtBlockRef() const
{
    switch (type) {
            
        case FSBlockType::ROOT_BLOCK:        return get32(-24);
        case FSBlockType::BITMAP_EXT_BLOCK:  return get32(-1);
            
        default:
            return 0;
    }
}

void
FSBlock::setNextBmExtBlockRef(Block ref)
{
    switch (type) {
            
        case FSBlockType::ROOT_BLOCK:        set32(-24, ref); break;
        case FSBlockType::BITMAP_EXT_BLOCK:  set32(-1, ref); break;
            
        default:
            break;
    }
}

FSBlock *
FSBlock::getNextBmExtBlock()
{
    Block nr = getNextBmExtBlockRef();
    return nr ? device.bitmapExtBlockPtr(nr) : nullptr;
}

Block
FSBlock::getFirstDataBlockRef() const
{
    switch (type) {
            
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
            
            return get32(4);
            
        default:
            return 0;
    }
}

void
FSBlock::setFirstDataBlockRef(Block ref)
{
    switch (type) {

        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
            
            set32(4, ref);
            break;
            
        default:
            break;
    }
}

FSBlock *
FSBlock::getFirstDataBlock()
{
    Block nr = getFirstDataBlockRef();
    return nr ? device.dataBlockPtr(nr) : nullptr;
}

Block
FSBlock::getDataBlockRef(isize nr) const
{
    switch (type) {
            
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
            
            return get32(-51 - nr);
            
        default:
            fatalError;
    }
}

void
FSBlock::setDataBlockRef(isize nr, Block ref)
{
    switch (type) {
            
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
            
            set32(-51-nr, ref);
            return;
            
        default:
            fatalError;
    }
}

Block
FSBlock::getNextDataBlockRef() const
{
    return type == FSBlockType::DATA_BLOCK_OFS ? get32(4) : 0;
}

void
FSBlock::setNextDataBlockRef(Block ref)
{
    if (type == FSBlockType::DATA_BLOCK_OFS) set32(4, ref);
}

FSBlock *
FSBlock::getNextDataBlock()
{
    Block nr = getNextDataBlockRef();
    return nr ? device.dataBlockPtr(nr) : nullptr;
}

isize
FSBlock::hashTableSize() const
{
    switch (type) {
            
        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
            
            return 72;
            
        default:
            return 0;
    }
}

u32
FSBlock::hashValue() const
{
    switch (type) {
            
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
            
            return getName().hashValue();
            
        default:
            return 0;
    }
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
        
        u32 value = read32(data.ptr + 24 + 4 * i);
        if (value) {
            msg("%ld: %d ", i, value);
        }
    }
}

void
FSBlock::writeBootBlock(BootBlockId id, isize page)
{
    assert(page == 0 || page == 1);
    assert(type == FSBlockType::BOOT_BLOCK);
    
    debug(FS_DEBUG, "writeBootBlock(%s, %ld)\n", BootBlockIdEnum::key(id), page);
    
    if (id != BootBlockId::NONE) {
        
        // Read boot block image from the database
        auto image = BootBlockImage(id);
        
        if (page == 0) {
            image.write(data.ptr + 4, 4, 511); // Write 508 bytes (skip header)
        } else {
            image.write(data.ptr, 512, 1023);  // Write 512 bytes
        }
    }
}

bool
FSBlock::addBitmapBlockRefs(std::vector<Block> &refs)
{
    assert(type == FSBlockType::ROOT_BLOCK);
    
    auto it = refs.begin();

    // Record the first 25 references in the root block
    for (isize i = 0; i < 25; i++, it++) {
        if (it == refs.end()) return true;
        setBmBlockRef(i, *it);
    }

    // Record the remaining references in bitmap extension blocks
    FSBlock *ext = getNextBmExtBlock();
    while (ext && it != refs.end()) {
        ext->addBitmapBlockRefs(refs, it);
        ext = getNextBmExtBlock();
    }
    
    return it == refs.end();
}

void
FSBlock::addBitmapBlockRefs(std::vector<Block> &refs,
                            std::vector<Block>::iterator &it)
{
    assert(type == FSBlockType::BITMAP_EXT_BLOCK);
    
    isize max = (bsize() / 4) - 1;
    
    for (isize i = 0; i < max; i++, it++) {
        if (it == refs.end()) return;
        setBmBlockRef(i, *it);
    }
}

Block
FSBlock::getBmBlockRef(isize nr) const
{
    switch (type) {
            
        case FSBlockType::ROOT_BLOCK:
            
            return get32(nr - 49);
            
        case FSBlockType::BITMAP_EXT_BLOCK:
            
            return get32(nr);
            
        default:
            fatalError;
    }
}

void
FSBlock::setBmBlockRef(isize nr, Block ref)
{
    switch (type) {
            
        case FSBlockType::ROOT_BLOCK:
            
            set32(nr - 49, ref);
            return;
            
        case FSBlockType::BITMAP_EXT_BLOCK:
            
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
            
        case FSBlockType::DATA_BLOCK_OFS: return get32(2);
        case FSBlockType::DATA_BLOCK_FFS: return 0;

        default:
            fatalError;
    }
}

void
FSBlock::setDataBlockNr(u32 val)
{
    switch (type) {
            
        case FSBlockType::DATA_BLOCK_OFS: set32(2, val); break;
        case FSBlockType::DATA_BLOCK_FFS: break;

        default:
            fatalError;
    }
}

isize
FSBlock::getMaxDataBlockRefs() const
{
    return bsize() / 4 - 56;
}

isize
FSBlock::getNumDataBlockRefs() const
{
    switch (type) {
            
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
            
            return get32(2);

        default:
            return 0;
    }
}

void
FSBlock::setNumDataBlockRefs(u32 val)
{
    switch (type) {
            
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:

            set32(2, val);
            break;

        default:
            break;
    }
}

void
FSBlock::incNumDataBlockRefs()
{
    switch (type) {
            
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:

            inc32(2);
            break;

        default:
            break;
    }
}

bool
FSBlock::addDataBlockRef(u32 first, u32 ref)
{
    switch (type) {
            
        case FSBlockType::FILEHEADER_BLOCK:
        {
            std::set<Block> visited;
            
            // If this block has space for more references, add it here
            if (getNumDataBlockRefs() < getMaxDataBlockRefs()) {
                
                if (getNumDataBlockRefs() == 0) setFirstDataBlockRef(first);
                setDataBlockRef(getNumDataBlockRefs(), ref);
                incNumDataBlockRefs();
                return true;
            }
            
            // Otherwise, add it to an extension block
            FSBlock *item = getNextListBlock();
            
            while (item) {
                
                // Break the loop if we visit a block twice
                if (visited.find(item->nr) != visited.end()) return false;
                
                if (item->addDataBlockRef(first, ref)) return true;
                item = item->getNextListBlock();
            }
            
            return false;
        }

        case FSBlockType::FILELIST_BLOCK:
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
            
        default:
            return false;
    }
}

u32
FSBlock::getDataBytesInBlock() const
{
    switch (type) {
            
        case FSBlockType::DATA_BLOCK_OFS: return get32(3);
        case FSBlockType::DATA_BLOCK_FFS: return 0;

        default:
            fatalError;
    }
}

void
FSBlock::setDataBytesInBlock(u32 val)
{
    switch (type) {
            
        case FSBlockType::DATA_BLOCK_OFS: set32(3, val); break;
        case FSBlockType::DATA_BLOCK_FFS: break;

        default:
            fatalError;
    }
}

isize
FSBlock::writeData(std::ostream& os)
{
    Buffer<u8> buffer;
    
    // Write block into buffer
    writeData(buffer);
    
    // Export the buffer
    os << buffer;
    
    return buffer.size;
}

isize
FSBlock::writeData(std::ostream& os, isize size)
{
    isize count = std::min(dsize(), size);
    
    switch (type) {
            
        case FSBlockType::DATA_BLOCK_OFS:
            
            os.write((char *)(data.ptr + 24), count);
            return count;
            
        case FSBlockType::DATA_BLOCK_FFS:
            
            os.write((char *)data.ptr, count);
            return count;
            
        default:
            fatalError;
    }
}

isize
FSBlock::writeData(Buffer<u8> &buf)
{
    // Only call this function for file header blocks
    assert(type == FSBlockType::FILEHEADER_BLOCK);
    
    isize bytesRemaining = getFileSize();
    isize bytesTotal = 0;
    isize blocksTotal = 0;
    
    buf.init(bytesRemaining);
    
    // Start here and iterate through all connected file list blocks
    FSBlock *block = this;
    
    while (block && blocksTotal < device.numBlocks()) {
        
        blocksTotal++;
        
        // Iterate through all data blocks references in this block
        isize num = std::min(block->getNumDataBlockRefs(), block->getMaxDataBlockRefs());
        for (isize i = 0; i < num; i++) {
            
            Block ref = block->getDataBlockRef(i);
            if (FSBlock *dataBlock = device.dataBlockPtr(ref)) {

                isize bytesWritten = dataBlock->writeData(buf, bytesTotal, bytesRemaining);
                bytesTotal += bytesWritten;
                bytesRemaining -= bytesWritten;
                
            } else {
                
                warn("Ignoring block %d (no data block)\n", ref);
            }
        }
        
        // Continue with the next list block
        block = block->getNextListBlock();
    }
    
    if (bytesRemaining != 0) {
        warn("%ld remaining bytes. Expected 0.\n", bytesRemaining);
    }
    
    return bytesTotal;
}

isize
FSBlock::writeData(Buffer<u8> &buf, isize offset, isize count)
{
    count = std::min(dsize(), count);
    
    switch (type) {
            
        case FSBlockType::DATA_BLOCK_OFS:
            
            std::memcpy((void *)(buf.ptr + offset), (void *)(data.ptr + 24), count);
            return count;
            
        case FSBlockType::DATA_BLOCK_FFS:

            std::memcpy((void *)(buf.ptr + offset), (void *)(data.ptr), count);
            return count;
            
        default:
            fatalError;
    }
}

isize
FSBlock::overwriteData(Buffer<u8> &buf)
{
    // Only call this function for file header blocks
    assert(type == FSBlockType::FILEHEADER_BLOCK);
    
    isize bytesRemaining = getFileSize();
    isize bytesTotal = 0;
    isize blocksTotal = 0;
    
    assert(buf.size == bytesRemaining);
    
    // Start here and iterate through all connected file list blocks
    FSBlock *block = this;
    
    while (block && blocksTotal < device.numBlocks()) {
        
        blocksTotal++;
        
        // Iterate through all data blocks references in this block
        isize num = std::min(block->getNumDataBlockRefs(), block->getMaxDataBlockRefs());
        for (isize i = 0; i < num; i++) {

            Block ref = block->getDataBlockRef(i);
            if (FSBlock *dataBlock = device.dataBlockPtr(ref)) {
                
                isize bytesWritten = dataBlock->overwriteData(buf, bytesTotal, bytesRemaining);
                bytesTotal += bytesWritten;
                bytesRemaining -= bytesWritten;
                
            } else {
                
                warn("Ignoring block %d (no data block)\n", ref);
            }
        }
        
        // Continue with the next list block
        block = block->getNextListBlock();
    }
    
    if (bytesRemaining != 0) {
        warn("%ld remaining bytes. Expected 0.\n", bytesRemaining);
    }
    
    return bytesTotal;
}

isize
FSBlock::overwriteData(Buffer<u8> &buf, isize offset, isize count)
{
    count = std::min(dsize(), count);
    
    switch (type) {
            
        case FSBlockType::DATA_BLOCK_OFS:
            
            std::memcpy((void *)(data.ptr + 24), (void *)(buf.ptr + offset), count);
            return count;
            
        case FSBlockType::DATA_BLOCK_FFS:

            std::memcpy((void *)(data.ptr), (void *)(buf.ptr + offset), count);
            return count;
            
        default:
            fatalError;
    }
}

}
