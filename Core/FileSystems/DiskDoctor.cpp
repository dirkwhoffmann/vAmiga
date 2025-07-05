/// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "DiskDoctor.h"
#include "FileSystem.h"
#include <unordered_map>
#include <unordered_set>

//
// Macros used inside the check() methods
//

#define EXPECT_BYTE(exp) { \
if (value != (exp)) { *expected = (exp); return Fault::FS_EXPECTED_VALUE; } }

#define EXPECT_LONGWORD(exp) { \
if ((pos % 4) == 0 && BYTE3(value) != BYTE3((u32)exp)) \
{ *expected = (BYTE3((u32)exp)); return Fault::FS_EXPECTED_VALUE; } \
if ((pos % 4) == 1 && BYTE2(value) != BYTE2((u32)exp)) \
{ *expected = (BYTE2((u32)exp)); return Fault::FS_EXPECTED_VALUE; } \
if ((pos % 4) == 2 && BYTE1(value) != BYTE1((u32)exp)) \
{ *expected = (BYTE1((u32)exp)); return Fault::FS_EXPECTED_VALUE; } \
if ((pos % 4) == 3 && BYTE0(value) != BYTE0((u32)exp)) \
{ *expected = (BYTE0((u32)exp)); return Fault::FS_EXPECTED_VALUE; } }

#define EXPECT_CHECKSUM EXPECT_LONGWORD(node.checksum())

#define EXPECT_LESS_OR_EQUAL(exp) { \
if (value > (u32)exp) \
{ *expected = (u8)(exp); return Fault::FS_EXPECTED_SMALLER_VALUE; } }

#define EXPECT_DOS_REVISION { \
if (!FSVolumeTypeEnum::isValid((isize)value)) return Fault::FS_EXPECTED_DOS_REVISION; }

#define EXPECT_REF { \
if (!fs.block(value)) return Fault::FS_EXPECTED_REF; }

#define EXPECT_SELFREF { \
if (value != ref) return Fault::FS_EXPECTED_SELFREF; }

#define EXPECT_FILEHEADER_REF { \
if (Fault e = checkBlockType(value, FSBlockType::FILEHEADER_BLOCK); e != Fault::OK) return e; }

#define EXPECT_HASH_REF { \
if (Fault e = checkBlockType(value, FSBlockType::FILEHEADER_BLOCK, FSBlockType::USERDIR_BLOCK); e != Fault::OK) return e; }

#define EXPECT_OPTIONAL_HASH_REF { \
if (value) { EXPECT_HASH_REF } }

#define EXPECT_PARENT_DIR_REF { \
if (Fault e = checkBlockType(value, FSBlockType::ROOT_BLOCK, FSBlockType::USERDIR_BLOCK); e != Fault::OK) return e; }

#define EXPECT_FILELIST_REF { \
if (Fault e = checkBlockType(value, FSBlockType::FILELIST_BLOCK); e != Fault::OK) return e; }

#define EXPECT_OPTIONAL_FILELIST_REF { \
if (value) { EXPECT_FILELIST_REF } }

#define EXPECT_BITMAP_REF { \
if (Fault e = checkBlockType(value, FSBlockType::BITMAP_BLOCK); e != Fault::OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_REF { \
if (value) { EXPECT_BITMAP_REF } }

#define EXPECT_BITMAP_EXT_REF { \
if (Fault e = checkBlockType(value, FSBlockType::BITMAP_EXT_BLOCK); e != Fault::OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_EXT_REF { \
if (value) { EXPECT_BITMAP_EXT_REF } }

#define EXPECT_DATABLOCK_REF { \
if (Fault e = checkBlockType(value, FSBlockType::DATA_BLOCK_OFS, FSBlockType::DATA_BLOCK_FFS); e != Fault::OK) return e; }

#define EXPECT_OPTIONAL_DATABLOCK_REF { \
if (value) { EXPECT_DATABLOCK_REF } }

#define EXPECT_DATABLOCK_NUMBER { \
if (value == 0) return Fault::FS_EXPECTED_DATABLOCK_NR; }

#define EXPECT_HASHTABLE_SIZE { \
if (value != 72) return Fault::FS_INVALID_HASHTABLE_SIZE; }

namespace vamiga {

void
DiskDoctor::dump(Block nr, std::ostream &os)
{
    using namespace util;

    FSBlock &p = fs.at(nr);

    os << tab("Block");
    os << dec(nr) << std::endl;
    os << tab("Type");
    os << FSBlockTypeEnum::key(p.type) << std::endl;

    if (p.hasHeaderKey()) {

        os << tab("Header Key");
        os << dec(p.getHeaderKey()) << std::endl;
    }
    if (p.hasChecksum()) {

        os << tab("Checksum");
        os << hex(p.getChecksum()) << std::endl;
    }

    switch (p.type) {

        case FSBlockType::BOOT_BLOCK:

            os << tab("Header");
            for (isize i = 0; i < 8; i++) os << hex(p.bdata[i]) << " ";
            os << std::endl;
            break;

        case FSBlockType::ROOT_BLOCK:

            os << tab("Name");
            os << p.getName() << std::endl;
            os << tab("Created");
            os << p.getCreationDate().str() << std::endl;
            os << tab("Modified");
            os << p.getCreationDate().str() << std::endl;
            os << tab("Bitmap blocks");
            os << FSBlock::rangeString(p.getBmBlockRefs()) << std::endl;
            os << tab("Bitmap extension block");
            os << dec(p.getNextBmExtBlockRef()) << std::endl;
            break;

        case FSBlockType::BITMAP_BLOCK:
        {
            isize count = 0;
            for (isize i = 1; i < p.bsize() / 4; i++) {
                if (u32 value = p.get32(i)) {
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
            os << FSBlock::rangeString(p.getBmBlockRefs()) << std::endl;
            os << tab("Next extension block");
            os << dec(p.getNextBmExtBlockRef()) << std::endl;
            break;

        case FSBlockType::USERDIR_BLOCK:

            os << tab("Name");
            os << p.getName() << std::endl;
            os << tab("Comment");
            os << p.getComment() << std::endl;
            os << tab("Created");
            os << p.getCreationDate().str() << std::endl;
            os << tab("Parent");
            os << dec(p.getParentDirRef()) << std::endl;
            os << tab("Next");
            os << dec(p.getNextHashRef()) << std::endl;
            break;

        case FSBlockType::FILEHEADER_BLOCK:

            os << tab("Name");
            os << p.getName() << std::endl;
            os << tab("Comment");
            os << p.getComment() << std::endl;
            os << tab("Created");
            os << p.getCreationDate().str() << std::endl;
            os << tab("UID (User ID)");
            os << hex(HI_WORD(p.get32(-49))) << std::endl;
            os << tab("GID (Group ID)");
            os << hex(LO_WORD(p.get32(-49))) << std::endl;
            os << tab("Protection flags");
            os << hex(p.getProtectionBits()) << std::endl;
            os << tab("File size");
            os << dec(p.getFileSize()) << " bytes" << std::endl;
            os << tab("First data block");
            os << dec(p.getFirstDataBlockRef()) << std::endl;
            os << tab("Data block count");
            os << dec(p.getNumDataBlockRefs()) << " out of " << dec(p.getMaxDataBlockRefs()) << std::endl;
            os << tab("Data block refs");
            os << FSBlock::rangeString(p.getDataBlockRefs()) << std::endl;
            os << tab("First extension block");
            os << dec(p.getNextListBlockRef()) << std::endl;
            os << tab("Parent dir");
            os << dec(p.getParentDirRef()) << std::endl;
            os << tab("Next file");
            os << dec(p.getNextHashRef()) << std::endl;
            break;

        case FSBlockType::FILELIST_BLOCK:

            os << tab("Header block");
            os << p.getFileHeaderRef() << std::endl;
            os << tab("Data block count");
            os << p.getNumDataBlockRefs() << " out of " << p.getMaxDataBlockRefs() << std::endl;
            os << tab("First");
            os << p.getFirstDataBlockRef() << std::endl;
            os << tab("Data blocks");
            os << FSBlock::rangeString(p.getDataBlockRefs()) << std::endl;
            os << tab("Next extension block");
            os << p.getNextListBlockRef() << std::endl;
            break;

        case FSBlockType::DATA_BLOCK_OFS:

            os << tab("File header block");
            os << p.getFileHeaderRef() << std::endl;
            os << tab("Chain number");
            os << p.getDataBlockNr() << std::endl;
            os << tab("Data bytes");
            os << p.getDataBytesInBlock() << std::endl;
            os << tab("Next data block");
            os << p.getNextDataBlockRef() << std::endl;
            break;

        default:
            break;
    }

    if (p.hashTableSize() > 0) {

        os << tab("Hash table");
        for (isize i = 0, j = 0; i < p.hashTableSize(); i++) {

            if (Block ref = p.read32(p.bdata + 24 + 4 * i); ref) {

                if (j++) os << std::endl << tab();
                os << std::setfill(' ') << std::setw(2) << i << " -> ";
                os << std::setfill(' ') << std::setw(4) << ref;

                if (auto ptr = fs.read(ref); ptr) {
                    os << " (" << ptr->getName().cpp_str() << ")";
                }
            }
        }
        os << std::endl;
    }
}

isize
DiskDoctor::xray(bool strict)
{
    return xrayBlocks(strict) + xrayBitmap(strict);
}

isize
DiskDoctor::xray(std::ostream &os, bool strict)
{
    return xrayBlocks(os, strict) + xrayBitmap(os, strict);
}

isize
DiskDoctor::xrayBlocks(bool strict)
{
    diagnosis.blockErrors = {};

    for (isize i = 0, capacity = fs.numBlocks(); i < capacity; i++) {

        if (xray(Block(i), strict)) diagnosis.blockErrors.push_back(Block(i));
    }

    return (isize)diagnosis.blockErrors.size();

    // return { 12, 24, 42,43,44, 67,69};
}

isize
DiskDoctor::xrayBlocks(std::ostream &os, bool strict)
{
    auto result = xrayBlocks(strict);

    auto blocks = [&](size_t s) { return std::to_string(s) + (s == 1 ? " block" : " blocks"); };

    if (auto total = diagnosis.blockErrors.size(); total) {

        os << util::tab("Block anomalies:");
        os << blocks(total) << std::endl;
        os << util::tab("Corrupted blocks:");
        os << FSBlock::rangeString(diagnosis.blockErrors) << std::endl;
    }
    return result;
}

isize
DiskDoctor::xrayBitmap(bool strict)
{
    // std::unordered_map<Block,isize> result;
    std::unordered_set<Block> used;

    // Extract the directory tree
    auto tree = FSTree(fs.root(), { .recursive = true });

    // Collect all used blocks
    tree.bfsWalk( [&](const FSTree &it) {

        used.insert(it.node->nr);

        if (it.node->isFile()) {

            auto listBlocks = fs.collectListBlocks(it.node->nr);
            auto dataBlocks = fs.collectDataBlocks(it.node->nr);
            used.insert(listBlocks.begin(), listBlocks.end());
            used.insert(dataBlocks.begin(), dataBlocks.end());
        }
    });
    used.insert(fs.bmBlocks.begin(), fs.bmBlocks.end());
    used.insert(fs.bmExtBlocks.begin(), fs.bmExtBlocks.end());

    // Check all blocks (ignoring the first two boot blocks)
    for (isize i = 2, capacity = fs.numBlocks(); i < capacity; i++) {

        bool allocated = fs.isAllocated(Block(i));
        bool contained = used.contains(Block(i));

        if (allocated && !contained) {

            diagnosis.unusedButAllocated.push_back(Block(i));
            diagnosis.bitmapErrors[Block(i)] = 1;

        } else if (!allocated && contained) {

            diagnosis.usedButUnallocated.push_back(Block(i));
            diagnosis.bitmapErrors[Block(i)] = 2;
        }
    }

    return (isize)diagnosis.bitmapErrors.size();
}

isize
DiskDoctor::xrayBitmap(std::ostream &os, bool strict)
{
    auto result = xrayBitmap(strict);

    auto &usedButUnallocated = fs.doctor.diagnosis.usedButUnallocated;
    auto &unusedButAllocated = fs.doctor.diagnosis.unusedButAllocated;

    auto blocks = [&](size_t s) { return std::to_string(s) + (s == 1 ? " block" : " blocks"); };

    if (auto total = usedButUnallocated.size() + unusedButAllocated.size(); total) {

        os << util::tab("Bitmap anomalies:") << blocks(total) << std::endl;

        if (!usedButUnallocated.empty()) {

            os << util::tab("Used but unallocated:");
            os << FSBlock::rangeString(usedButUnallocated) << std::endl;
        }
        if (!unusedButAllocated.empty()) {

            os << util::tab("Allocated but unused:");
            os << FSBlock::rangeString(unusedButAllocated) << std::endl;
        }
    }
    return result;
}

isize
DiskDoctor::xray(Block ref, bool strict) const
{
    return xray(fs.at(ref), strict);
}

isize
DiskDoctor::xray(FSBlock &node, bool strict) const
{
    isize count = 0;

    for (isize i = 0; i < node.bsize(); i++) {

        u8 expected;
        if (Fault error = xray(node.nr, i, strict, &expected); error != Fault::OK) {

            count++;
            debug(FS_DEBUG, "Block %d [%ld.%ld]: %s\n", node.nr, i / 4, i % 4, FaultEnum::key(error));
        }
    }

    return count;
}

Fault
DiskDoctor::xray(Block ref, isize pos, bool strict) const
{
    return xray(fs.at(ref), pos, strict);
}

Fault
DiskDoctor::xray(Block ref, isize pos, bool strict, u8 *expected) const
{
    return xray(fs.at(ref), pos, strict, expected);
}

Fault
DiskDoctor::xray(FSBlock &node, isize pos, bool strict) const
{
    u8 expected;
    return xray(node, pos, strict, &expected);
}

Fault
DiskDoctor::xray(FSBlock &node, isize pos, bool strict, u8 *expected) const
{
    auto ref = node.nr;

    switch (node.type) {

        case FSBlockType::BOOT_BLOCK:
        {
            isize word = pos / 4;
            u32 value = node.bdata[pos];

            if (ref == 0) {

                if (pos == 0) EXPECT_BYTE('D');
                if (pos == 1) EXPECT_BYTE('O');
                if (pos == 2) EXPECT_BYTE('S');
                if (pos == 3) EXPECT_DOS_REVISION;
                if (word == 1) { value = node.get32(1); EXPECT_CHECKSUM; }
            }
            break;
        }
        case FSBlockType::ROOT_BLOCK:
        {
            isize word = pos / 4; if (word >= 6) word -= node.bsize() / 4;
            u32 value = node.get32(word);

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
            isize word = pos / 4;
            u32 value = node.get32(word);

            if (word == 0) EXPECT_CHECKSUM;
            break;
        }
        case FSBlockType::BITMAP_EXT_BLOCK:
        {
            isize word = pos / 4;
            u32 value = node.get32(word);

            if (word == (i32)(node.bsize() - 4)) EXPECT_OPTIONAL_BITMAP_EXT_REF;
            break;
        }
        case FSBlockType::USERDIR_BLOCK:
        {
            isize word = pos / 4; if (word >= 6) word -= node.bsize() / 4;
            u32 value = node.get32(word);

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
            isize word = pos / 4; if (word >= 6) word -= node.bsize() / 4;
            u32 value = node.get32(word);

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
                if (value == 0 && node.getNumDataBlockRefs() > 0) {
                    return Fault::FS_EXPECTED_REF;
                }
                if (value != 0 && node.getNumDataBlockRefs() == 0) {
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
            isize word = pos / 4; if (word >= 6) word -= node.bsize() / 4;
            u32 value = node.get32(word);

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
                if (value == 0 && node.getNumDataBlockRefs() > 0) {
                    return Fault::FS_EXPECTED_REF;
                }
                if (value != 0 && node.getNumDataBlockRefs() == 0) {
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

            if (pos < 24) {

                isize word = pos / 4;
                u32 value = node.get32(word);

                switch (word) {

                    case 0: EXPECT_LONGWORD(8);                  break;
                    case 1: if (strict) EXPECT_FILEHEADER_REF;   break;
                    case 2: EXPECT_DATABLOCK_NUMBER;             break;
                    case 3: EXPECT_LESS_OR_EQUAL(node.dsize()); break;
                    case 4: EXPECT_OPTIONAL_DATABLOCK_REF;       break;
                    case 5: EXPECT_CHECKSUM;                     break;
                }
            }
            break;
        }

        default:
            break;
    }

    return Fault::OK;
}

Fault
DiskDoctor::checkBlockType(Block nr, FSBlockType type) const
{
    return checkBlockType(nr, type, type);
}

Fault
DiskDoctor::checkBlockType(Block nr, FSBlockType type, FSBlockType altType) const
{
    auto t = fs.typeof(nr);

    if (t != type && t != altType) {

        switch (t) {

            case FSBlockType::EMPTY_BLOCK:      return Fault::FS_PTR_TO_EMPTY_BLOCK;
            case FSBlockType::BOOT_BLOCK:       return Fault::FS_PTR_TO_BOOT_BLOCK;
            case FSBlockType::ROOT_BLOCK:       return Fault::FS_PTR_TO_ROOT_BLOCK;
            case FSBlockType::BITMAP_BLOCK:     return Fault::FS_PTR_TO_BITMAP_BLOCK;
            case FSBlockType::BITMAP_EXT_BLOCK: return Fault::FS_PTR_TO_BITMAP_EXT_BLOCK;
            case FSBlockType::USERDIR_BLOCK:    return Fault::FS_PTR_TO_USERDIR_BLOCK;
            case FSBlockType::FILEHEADER_BLOCK: return Fault::FS_PTR_TO_FILEHEADER_BLOCK;
            case FSBlockType::FILELIST_BLOCK:   return Fault::FS_PTR_TO_FILELIST_BLOCK;
            case FSBlockType::DATA_BLOCK_OFS:   return Fault::FS_PTR_TO_DATA_BLOCK;
            case FSBlockType::DATA_BLOCK_FFS:   return Fault::FS_PTR_TO_DATA_BLOCK;
            default:                            return Fault::FS_PTR_TO_UNKNOWN_BLOCK;
        }
    }

    return Fault::OK;
}

}
