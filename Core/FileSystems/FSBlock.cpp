// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSBlock.h"
#include "MutableFileSystem.h"
#include "MemUtils.h"
#include "Host.h"

namespace vamiga {

FSBlock::FSBlock(FileSystem &ref, Block nr, FSBlockType t) : fs(ref)
{
    this->nr = nr;
    init(t);
}

void
FSBlock::init(FSBlockType t)
{
    type = t;
    
    if (type == FSBlockType::UNKNOWN_BLOCK) return;
        
    // Allocate memory
    data.init(type == FSBlockType::EMPTY_BLOCK ? 0 : bsize(), 0);
    
    // Initialize
    switch (type) {

        case FSBlockType::BOOT_BLOCK:
            
            if (nr == 0 && fs.dos != FSVolumeType::NODOS) {
                data[0] = 'D';
                data[1] = 'O';
                data[2] = 'S';
                data[3] = (u8)fs.dos;
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
            throw AppError(Fault::FS_INVALID_BLOCK_TYPE);
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
            throw AppError(Fault::FS_INVALID_BLOCK_TYPE);
    }
}

isize
FSBlock::bsize() const
{
    return fs.bsize;
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
    u8 *p = fs.blocks[1]->data.ptr;
    
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
FSBlock::dump(std::ostream &os) const
{
    using namespace util;

    os << tab("Block");
    os << dec(nr) << std::endl;
    os << tab("Type");
    os << FSBlockTypeEnum::key(type) << std::endl;

    if (hasHeaderKey()) {

        os << tab("Header Key");
        os << dec(getHeaderKey()) << std::endl;
    }
    if (hasChecksum()) {

        os << tab("Checksum");
        os << hex(getChecksum()) << std::endl;
    }

    switch (type) {

        case FSBlockType::BOOT_BLOCK:

            os << tab("Header");
            for (isize i = 0; i < 8; i++) os << hex(data[i]) << " ";
            os << std::endl;
            break;

        case FSBlockType::ROOT_BLOCK:

            os << tab("Name");
            os << getName() << std::endl;
            os << tab("Created");
            os << getCreationDate().str() << std::endl;
            os << tab("Modified");
            os << getCreationDate().str() << std::endl;
            os << tab("Bitmap blocks");
            os << FSBlock::rangeString(getBmBlockRefs()) << std::endl;
            os << tab("Bitmap extension block");
            os << dec(getNextBmExtBlockRef()) << std::endl;
            break;

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
            os << tab("Free");
            os << dec(count) << " blocks" << std::endl;
            break;
        }
        case FSBlockType::BITMAP_EXT_BLOCK:

            os << tab("Bitmap blocks");
            os << FSBlock::rangeString(getBmBlockRefs()) << std::endl;
            os << tab("Next extension block");
            os << dec(getNextBmExtBlockRef()) << std::endl;
            break;

        case FSBlockType::USERDIR_BLOCK:

            os << tab("Name");
            os << getName() << std::endl;
            os << tab("Comment");
            os << getComment() << std::endl;
            os << tab("Created");
            os << getCreationDate().str() << std::endl;
            os << tab("Parent");
            os << dec(getParentDirRef()) << std::endl;
            os << tab("Next");
            os << dec(getNextHashRef()) << std::endl;
            break;
            
        case FSBlockType::FILEHEADER_BLOCK:

            os << tab("Name");
            os << getName() << std::endl;
            os << tab("Comment");
            os << getComment() << std::endl;
            os << tab("Created");
            os << getCreationDate().str() << std::endl;
            os << tab("UID (User ID)");
            os << hex(HI_WORD(get32(-49))) << std::endl;
            os << tab("GID (Group ID)");
            os << hex(LO_WORD(get32(-49))) << std::endl;
            os << tab("Protection flags");
            os << hex(getProtectionBits()) << std::endl;
            os << tab("File size");
            os << dec(getFileSize()) << " bytes" << std::endl;
            os << tab("First data block");
            os << dec(getFirstDataBlockRef()) << std::endl;
            os << tab("Data block count");
            os << dec(getNumDataBlockRefs()) << " out of " << dec(getMaxDataBlockRefs()) << std::endl;
            os << tab("Data block refs");
            os << FSBlock::rangeString(getDataBlockRefs()) << std::endl;
            os << tab("First extension block");
            os << dec(getNextListBlockRef()) << std::endl;
            os << tab("Parent dir");
            os << dec(getParentDirRef()) << std::endl;
            os << tab("Next file");
            os << dec(getNextHashRef()) << std::endl;
            break;

        case FSBlockType::FILELIST_BLOCK:

            os << tab("Header block");
            os << getFileHeaderRef() << std::endl;
            os << tab("Data block count");
            os << getNumDataBlockRefs() << " out of " << getMaxDataBlockRefs() << std::endl;
            os << tab("First");
            os << getFirstDataBlockRef() << std::endl;
            os << tab("Data blocks");
            os << FSBlock::rangeString(getDataBlockRefs()) << std::endl;
            os << tab("Next extension block");
            os << getNextListBlockRef() << std::endl;
            break;

        case FSBlockType::DATA_BLOCK_OFS:

            os << tab("File header block");
            os << getFileHeaderRef() << std::endl;
            os << tab("Chain number");
            os << getDataBlockNr() << std::endl;
            os << tab("Data bytes");
            os << getDataBytesInBlock() << std::endl;
            os << tab("Next data block");
            os << getNextDataBlockRef() << std::endl;
            break;
            
        default:
            break;
    }

    if (hashTableSize() > 0) {

        os << tab("Hash table");
        for (isize i = 0, j = 0; i < hashTableSize(); i++) {

            if (Block ref = read32(data.ptr + 24 + 4 * i); ref) {

                if (j++) os << std::endl << tab();
                os << std::setfill(' ') << std::setw(2) << i << " -> ";
                os << std::setfill(' ') << std::setw(4) << ref;

                if (auto ptr = fs.blockPtr(ref); ptr) {
                    os << " (" << ptr->getName().cpp_str() << ")";
                }
            }
        }
        os << std::endl;
    }
}

void
FSBlock::hexDump(std::ostream &os, const util::DumpOpt &opt)
{
    data.dump(os, opt);
}

string
FSBlock::rangeString(std::vector<Block> vec)
{
    if (vec.empty()) return "";

    // Create a sorted copy
    std::vector<Block> v = vec;
    std::sort(v.begin(), v.end());

    // Replicate the last element to get the last interval right
    v.push_back(v.back());

    isize start = v[0], end = v[0];
    std::vector<string> chunks;

    for (usize i = 1; i < v.size(); i++) {

        if (v[i - 1] + 1 != v[i]) {

            if (start == end) chunks.push_back(std::to_string(start));
            else chunks.push_back(std::to_string(start) + " - " + std::to_string(end));
            start = v[i];
        }
        end = v[i];
    }

    return util::concat(chunks, ", ");
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
    // Assemble the host file name
    auto filename = path / FSPath(fs, nr).getPath();
    debug(FS_DEBUG >= 2, "Creating directory %s\n", filename.string().c_str());

    // Create directory
    if (!util::createDirectory(filename)) return Fault::FS_CANNOT_CREATE_DIR;

    return Fault::OK;
}

Fault
FSBlock::exportFileHeaderBlock(const fs::path &path)
{
    // Assemble the host file name
    auto filename = path / FSPath(fs, nr).getPath(); // device.getPath(this);
    debug(FS_DEBUG >= 2, "  Exporting file %s\n", filename.string().c_str());

    // Open file
    std::ofstream file(filename, std::ofstream::binary);
    if (!file.is_open()) return Fault::FS_CANNOT_CREATE_FILE;
    
    // Write data
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
FSBlock::isNamed(const FSName &other) const
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

bool
FSBlock::hasHeaderKey() const
{
    switch (type) {

        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
        case FSBlockType::DATA_BLOCK_OFS:

            return true;

        default:

            return false;
    }
}

u32
FSBlock::getHeaderKey() const
{
    switch (type) {

        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
        case FSBlockType::DATA_BLOCK_OFS:

            return get32(1);

        default:

            return 0;
    }
}

void
FSBlock::setHeaderKey(u32 val)
{
    switch (type) {

        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
        case FSBlockType::DATA_BLOCK_OFS:

            set32(1, val);

        default:

            break;
    }
}

bool
FSBlock::hasChecksum() const
{
    switch (type) {

        case FSBlockType::BOOT_BLOCK:

            return nr == 0;

        case FSBlockType::BITMAP_BLOCK:
        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
        case FSBlockType::DATA_BLOCK_OFS:

            return true;

        default:

            return false;
    }
}

u32
FSBlock::getChecksum() const
{
    switch (type) {

        case FSBlockType::BOOT_BLOCK:

            return nr == 0 ? get32(1) : 0;

        case FSBlockType::BITMAP_BLOCK:

            return get32(0);

        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
        case FSBlockType::DATA_BLOCK_OFS:

            return get32(5);

        default:

            return 0;
    }
}

void
FSBlock::setChecksum(u32 val)
{
    switch (type) {

        case FSBlockType::BOOT_BLOCK:

            if (nr == 0) set32(1, val);

        case FSBlockType::BITMAP_BLOCK:

            set32(0, val);

        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
        case FSBlockType::FILEHEADER_BLOCK:
        case FSBlockType::FILELIST_BLOCK:
        case FSBlockType::DATA_BLOCK_OFS:

            set32(5, val);

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
FSBlock::getParentDirBlock() const
{
    Block nr = getParentDirRef();
    return nr ? fs.blockPtr(nr) : nullptr;
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
FSBlock::getFileHeaderBlock() const
{
    Block nr = getFileHeaderRef();
    return nr ? fs.fileHeaderBlockPtr(nr) : nullptr;
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
FSBlock::getNextHashBlock() const
{
    Block nr = getNextHashRef();
    return nr ? fs.blockPtr(nr) : nullptr;
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
FSBlock::getNextListBlock() const
{
    Block nr = getNextListBlockRef();
    return nr ? fs.fileListBlockPtr(nr) : nullptr;
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
FSBlock::getNextBmExtBlock() const
{
    Block nr = getNextBmExtBlockRef();
    return nr ? fs.bitmapExtBlockPtr(nr) : nullptr;
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
            
            set32(4, ref);
            break;
            
        default:
            break;
    }
}

FSBlock *
FSBlock::getFirstDataBlock() const
{
    Block nr = getFirstDataBlockRef();
    return nr ? fs.dataBlockPtr(nr) : nullptr;
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
FSBlock::getNextDataBlock() const
{
    Block nr = getNextDataBlockRef();
    return nr ? fs.dataBlockPtr(nr) : nullptr;
}

isize
FSBlock::hashTableSize() const
{
    switch (type) {
            
        case FSBlockType::ROOT_BLOCK:
        case FSBlockType::USERDIR_BLOCK:
            
            assert(bsize() != 512 || (bsize() / 4) - 56 == 72);
            return (bsize() / 4) - 56;

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

isize
FSBlock::numBmBlockRefs() const
{
    switch (type) {

        case FSBlockType::ROOT_BLOCK:       return 25;
        case FSBlockType::BITMAP_EXT_BLOCK: return (bsize() / 4) - 1;

        default:
            return 0;
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

std::vector<Block>
FSBlock::getBmBlockRefs() const
{
    isize maxRefs =
    type == FSBlockType::ROOT_BLOCK ? 25 :
    type == FSBlockType::BITMAP_EXT_BLOCK ? (bsize() / 4) - 1 : 0;

    std::vector<Block> result;
    for (isize i = 0; i < maxRefs; i++) {
        if (auto ref = getBmBlockRef(i); ref) result.push_back(ref);
    }
    return result;
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

std::vector<Block>
FSBlock::getDataBlockRefs() const
{
    isize maxRefs = getNumDataBlockRefs();

    std::vector<Block> result;
    for (isize i = 0; i < maxRefs; i++) {
        if (auto ref = getDataBlockRef(i); ref) result.push_back(ref);
    }
    return result;
}

void
FSBlock::addDataBlockRef(u32 first, u32 ref)
{
    assert(getNumDataBlockRefs() < getMaxDataBlockRefs());
    
    switch (type) {
            
        case FSBlockType::FILEHEADER_BLOCK:
            
            setFirstDataBlockRef(first);
            setDataBlockRef(getNumDataBlockRefs(), ref);
            incNumDataBlockRefs();
            break;
            
        case FSBlockType::FILELIST_BLOCK:
            
            setDataBlockRef(getNumDataBlockRefs(), ref);
            incNumDataBlockRefs();
            break;
            
        default:
            
            break;
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
FSBlock::writeData(std::ostream &os)
{
    Buffer<u8> buffer;
    
    // Write block into buffer
    writeData(buffer);
    
    // Export the buffer
    os << buffer;
    
    return buffer.size;
}

isize
FSBlock::writeData(std::ostream &os, isize size)
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
    if (type != FSBlockType::FILEHEADER_BLOCK) throw AppError(Fault::FS_NOT_A_FILE);

    isize bytesRemaining = getFileSize();
    isize bytesTotal = 0;
    isize blocksTotal = 0;
    
    buf.init(bytesRemaining);
    
    // Start here and iterate through all connected file list blocks
    FSBlock *block = this;
    
    while (block && blocksTotal < fs.numBlocks()) {
        
        blocksTotal++;
        
        // Iterate through all data blocks references in this block
        isize num = std::min(block->getNumDataBlockRefs(), block->getMaxDataBlockRefs());
        for (isize i = 0; i < num; i++) {
            
            Block ref = block->getDataBlockRef(i);
            if (FSBlock *dataBlock = fs.dataBlockPtr(ref)) {

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
    
    while (block && blocksTotal < fs.numBlocks()) {
        
        blocksTotal++;
        
        // Iterate through all data blocks references in this block
        isize num = std::min(block->getNumDataBlockRefs(), block->getMaxDataBlockRefs());
        for (isize i = 0; i < num; i++) {

            Block ref = block->getDataBlockRef(i);
            if (FSBlock *dataBlock = fs.dataBlockPtr(ref)) {
                
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
