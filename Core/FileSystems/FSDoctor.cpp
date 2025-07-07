/// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FSDoctor.h"
#include "FileSystem.h"
#include <unordered_map>
#include <unordered_set>

//
// Macros used inside the check() methods
//

#define EXPECT_VALUE(exp) { \
if (value != u32(exp)) { expected = u32(exp); return FSBlockError::FS_EXPECTED_VALUE; } }

#define EXPECT_CHECKSUM EXPECT_VALUE(node.checksum())

#define EXPECT_LESS_OR_EQUAL(exp) { \
if (value > (u32)exp) \
{ expected = (u8)(exp); return FSBlockError::FS_EXPECTED_SMALLER_VALUE; } }

#define EXPECT_DOS_REVISION { \
if (!FSFormatEnum::isValid((isize)value)) return FSBlockError::FS_EXPECTED_DOS_REVISION; }

#define EXPECT_REF { \
if (!fs.block(value)) return FSBlockError::FS_EXPECTED_REF; }

#define EXPECT_SELFREF { \
if (value != ref) return FSBlockError::FS_EXPECTED_SELFREF; }

#define EXPECT_FILEHEADER_REF { \
if (auto e = checkBlockType(value, FSBlockType::FILEHEADER); e != FSBlockError::FS_OK) return e; }

#define EXPECT_HASH_REF { \
if (auto e = checkBlockType(value, FSBlockType::FILEHEADER, FSBlockType::USERDIR); e != FSBlockError::FS_OK) return e; }

#define EXPECT_OPTIONAL_HASH_REF { \
if (value) { EXPECT_HASH_REF } }

#define EXPECT_PARENT_DIR_REF { \
if (auto e = checkBlockType(value, FSBlockType::ROOT, FSBlockType::USERDIR); e != FSBlockError::FS_OK) return e; }

#define EXPECT_FILELIST_REF { \
if (auto e = checkBlockType(value, FSBlockType::FILELIST); e != FSBlockError::FS_OK) return e; }

#define EXPECT_OPTIONAL_FILELIST_REF { \
if (value) { EXPECT_FILELIST_REF } }

#define EXPECT_BITMAP_REF { \
if (auto e = checkBlockType(value, FSBlockType::BITMAP); e != FSBlockError::FS_OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_REF { \
if (value) { EXPECT_BITMAP_REF } }

#define EXPECT_BITMAP_EXT_REF { \
if (auto e = checkBlockType(value, FSBlockType::BITMAP_EXT); e != FSBlockError::FS_OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_EXT_REF { \
if (value) { EXPECT_BITMAP_EXT_REF } }

#define EXPECT_DATABLOCK_REF { \
if (auto e = checkBlockType(value, FSBlockType::DATA_OFS, FSBlockType::DATA_FFS); e != FSBlockError::FS_OK) return e; }

#define EXPECT_OPTIONAL_DATABLOCK_REF { \
if (value) { EXPECT_DATABLOCK_REF } }

#define EXPECT_DATABLOCK_NUMBER { \
if (value == 0) return FSBlockError::FS_EXPECTED_DATABLOCK_NR; }

#define EXPECT_HASHTABLE_SIZE { \
if (value != 72) return FSBlockError::FS_INVALID_HASHTABLE_SIZE; }

namespace vamiga {

void
FSDoctor::dump(Block nr, std::ostream &os)
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

        case FSBlockType::BOOT:

            os << tab("Header");
            for (isize i = 0; i < 8; i++) os << hex(p.bdata[i]) << " ";
            os << std::endl;
            break;

        case FSBlockType::ROOT:

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

        case FSBlockType::BITMAP:
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
        case FSBlockType::BITMAP_EXT:

            os << tab("Bitmap blocks");
            os << FSBlock::rangeString(p.getBmBlockRefs()) << std::endl;
            os << tab("Next extension block");
            os << dec(p.getNextBmExtBlockRef()) << std::endl;
            break;

        case FSBlockType::USERDIR:

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

        case FSBlockType::FILEHEADER:

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

        case FSBlockType::FILELIST:

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

        case FSBlockType::DATA_OFS:

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
FSDoctor::xray(bool strict)
{
    return xrayBlocks(strict) + xrayBitmap(strict);
}

isize
FSDoctor::xray(std::ostream &os, bool strict)
{
    return xrayBlocks(os, strict) + xrayBitmap(os, strict);
}

isize
FSDoctor::xrayBlocks(bool strict)
{
    diagnosis.blockErrors = {};

    for (isize i = 0, capacity = fs.numBlocks(); i < capacity; i++) {

        if (xray(Block(i), strict)) diagnosis.blockErrors.push_back(Block(i));
    }

    return (isize)diagnosis.blockErrors.size();

    // return { 12, 24, 42,43,44, 67,69};
}

isize
FSDoctor::xrayBlocks(std::ostream &os, bool strict)
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
FSDoctor::xrayBitmap(bool strict)
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
FSDoctor::xrayBitmap(std::ostream &os, bool strict)
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
FSDoctor::xray(Block ref, bool strict) const
{
    return xray(fs.at(ref), strict);
}

isize
FSDoctor::xray(FSBlock &node, bool strict) const
{
    isize count = 0;

    for (isize i = 0; i < node.bsize(); i++) {

        std::optional<u8> expected;
        if (auto error = xray(node.nr, i, strict, expected); error != FSBlockError::FS_OK) {

            count++;
            debug(FS_DEBUG, "Block %d [%ld.%ld]: %s\n", node.nr, i / 4, i % 4, FSBlockErrorEnum::key(error));
        }
    }

    return count;
}

FSBlockError
FSDoctor::xray(Block ref, isize pos, bool strict) const
{
    return xray(fs.at(ref), pos, strict);
}

FSBlockError
FSDoctor::xray(Block ref, isize pos, bool strict, optional<u8> &expected) const
{
    return xray(fs.at(ref), pos, strict, expected);
}

FSBlockError
FSDoctor::xray(FSBlock &node, isize pos, bool strict) const
{
    optional<u8> expected;
    return xray(node, pos, strict, expected);
}

FSBlockError
FSDoctor::xray(FSBlock &node, isize pos, bool strict, optional<u8> &expected) const
{
    optional<u32> exp;
    auto result = xray32(node, pos & ~3, strict, exp);
    if (exp) expected = GET_BYTE(*exp, 3 - (pos & 3));
    return result;
}

FSBlockError
FSDoctor::xray32(FSBlock &node, isize pos, bool strict) const
{
    optional<u32> expected;
    return xray32(node, pos, strict, expected);
}

FSBlockError
FSDoctor::xray32(FSBlock &node, isize pos, bool strict, optional<u32> &expected) const
{
    assert(pos % 4 == 0);

    isize word = pos / 4;
    auto ref = node.nr;

    switch (node.type) {

        case FSBlockType::BOOT:
        {
            u32 value = node.get32(word);

            if (ref == 0) {

                constexpr u32 DOS = u32('D') << 24 | u32('O') << 16 | u32('S') << 8;

                if (word == 0) { EXPECT_VALUE(DOS | u32(fs.traits.dos)); }
                if (word == 1) { value = node.get32(1); EXPECT_CHECKSUM; }
            }
            break;
        }
        case FSBlockType::ROOT:
        {
            if (word >= 6) word -= node.bsize() / 4;
            u32 value = node.get32(word);

            switch (word) {

                case 0:   EXPECT_VALUE(2);                break;
                case 1:
                case 2:   if (strict) EXPECT_VALUE(0);    break;
                case 3:   if (strict) EXPECT_HASHTABLE_SIZE; break;
                case 4:   EXPECT_VALUE(0);                break;
                case 5:   EXPECT_CHECKSUM;                   break;
                case -50:                                    break;
                case -49: EXPECT_BITMAP_REF;                 break;
                case -24: EXPECT_OPTIONAL_BITMAP_EXT_REF;    break;
                case -4:
                case -3:
                case -2:  if (strict) EXPECT_VALUE(0);    break;
                case -1:  EXPECT_VALUE(1);                break;

                default:

                    // Hash table area
                    if (word <= -51) { EXPECT_OPTIONAL_HASH_REF; break; }

                    // Bitmap block area
                    if (word <= -25) { EXPECT_OPTIONAL_BITMAP_REF; break; }
            }
            break;
        }
        case FSBlockType::BITMAP:
        {
            u32 value = node.get32(word);

            if (word == 0) EXPECT_CHECKSUM;
            break;
        }
        case FSBlockType::BITMAP_EXT:
        {
            u32 value = node.get32(word);

            if (word == (i32)(node.bsize() - 4)) EXPECT_OPTIONAL_BITMAP_EXT_REF;
            break;
        }
        case FSBlockType::USERDIR:
        {
            if (word >= 6) word -= node.bsize() / 4;
            u32 value = node.get32(word);

            switch (word) {
                case  0: EXPECT_VALUE(2);        break;
                case  1: EXPECT_SELFREF;            break;
                case  2:
                case  3:
                case  4: EXPECT_VALUE(0);            break;
                case  5: EXPECT_CHECKSUM;           break;
                case -4: EXPECT_OPTIONAL_HASH_REF;  break;
                case -3: EXPECT_PARENT_DIR_REF;     break;
                case -2: EXPECT_VALUE(0);            break;
                case -1: EXPECT_VALUE(2);        break;
            }
            if (word <= -51) EXPECT_OPTIONAL_HASH_REF;
            break;
        }
        case FSBlockType::FILEHEADER:
        {
            /* Note: At locations -4 and -3, many disks reference the bitmap
             * block which is wrong. We ignore to report this common
             * inconsistency if 'strict' is set to false.
             */

            // Translate the byte index to a (signed) long word index
            if (word >= 6) word -= node.bsize() / 4;
            u32 value = node.get32(word);

            switch (word) {
                case   0: EXPECT_VALUE(2);                    break;
                case   1: EXPECT_SELFREF;                        break;
                case   3: EXPECT_VALUE(0);                        break;
                case   4: EXPECT_DATABLOCK_REF;                  break;
                case   5: EXPECT_CHECKSUM;                       break;
                case -50: EXPECT_VALUE(0);                        break;
                case  -4: if (strict) EXPECT_OPTIONAL_HASH_REF;  break;
                case  -3: if (strict) EXPECT_PARENT_DIR_REF;     break;
                case  -2: EXPECT_OPTIONAL_FILELIST_REF;          break;
                case  -1: EXPECT_VALUE(-3);                   break;
            }

            // Data block reference area
            if (word <= -51 && value) EXPECT_DATABLOCK_REF;
            if (word == -51) {
                if (value == 0 && node.getNumDataBlockRefs() > 0) {
                    return FSBlockError::FS_EXPECTED_REF;
                }
                if (value != 0 && node.getNumDataBlockRefs() == 0) {
                    return FSBlockError::FS_EXPECTED_NO_REF;
                }
            }
            break;
        }
        case FSBlockType::FILELIST:
        {
            /* Note: At location -3, many disks reference the bitmap
             * block which is wrong. We ignore to report this common
             * inconsistency if 'strict' is set to false.
             */

            // Translate 'pos' to a (signed) long word index
            if (word >= 6) word -= node.bsize() / 4;
            u32 value = node.get32(word);

            switch (word) {

                case   0: EXPECT_VALUE(16);                break;
                case   1: EXPECT_SELFREF;                     break;
                case   3: EXPECT_VALUE(0);                     break;
                case   4: EXPECT_OPTIONAL_DATABLOCK_REF;      break;
                case   5: EXPECT_CHECKSUM;                    break;
                case -50:
                case  -4: EXPECT_VALUE(0);                     break;
                case  -3: if (strict) EXPECT_FILEHEADER_REF;  break;
                case  -2: EXPECT_OPTIONAL_FILELIST_REF;       break;
                case  -1: EXPECT_VALUE(-3);                break;
            }

            // Data block references
            if (word <= -51 && value) EXPECT_DATABLOCK_REF;
            if (word == -51) {
                if (value == 0 && node.getNumDataBlockRefs() > 0) {
                    return FSBlockError::FS_EXPECTED_REF;
                }
                if (value != 0 && node.getNumDataBlockRefs() == 0) {
                    return FSBlockError::FS_EXPECTED_NO_REF;
                }
            }
            break;
        }

        case FSBlockType::DATA_OFS:
        {
            /* Note: At location 1, many disks store a reference to the bitmap
             * block instead of a reference to the file header block. We ignore
             * to report this common inconsistency if 'strict' is set to false.
             */

            if (pos < 24) {

                u32 value = node.get32(word);

                switch (word) {

                    case 0: EXPECT_VALUE(8);                  break;
                    case 1: if (strict) EXPECT_FILEHEADER_REF;   break;
                    case 2: EXPECT_DATABLOCK_NUMBER;             break;
                    case 3: EXPECT_LESS_OR_EQUAL(node.dsize());  break;
                    case 4: EXPECT_OPTIONAL_DATABLOCK_REF;       break;
                    case 5: EXPECT_CHECKSUM;                     break;
                }
            }
            break;
        }

        default:
            break;
    }

    return FSBlockError::FS_OK;
}

isize
FSDoctor::xray(Block ref, std::ostream &os, bool strict) const
{
    return xray(fs.at(ref), os, strict);
}

isize
FSDoctor::xray(FSBlock &node, std::ostream &os, bool strict) const
{
    isize errors = 0;

    std::stringstream ss;

    auto hex = [&](int width, isize value, string delim = "") {
        ss << std::setw(width) << std::right << std::setfill('0') << std::hex << value << delim;
    };
    auto hex4 = [&](u32 value, string delim = "") {
        hex(2, BYTE3(value), " ");
        hex(2, BYTE2(value), " ");
        hex(2, BYTE1(value), " ");
        hex(2, BYTE0(value), delim);
    };

    auto describe = [&](FSBlockError fault, const optional<u32> &value) {

        switch (fault) {

            case FSBlockError::FS_OK:                       break;
            case FSBlockError::FS_EXPECTED_VALUE:          hex4(*value); break;
            case FSBlockError::FS_EXPECTED_SMALLER_VALUE:  ss << "< " << *value; break;
            case FSBlockError::FS_EXPECTED_NO_REF:
                ss << "Did not expect a block reference here"; break;
            case FSBlockError::FS_EXPECTED_REF:
                ss << "Expected a block reference"; break;
            case FSBlockError::FS_EXPECTED_SELFREF:
                ss << "Expected a self-reference"; break;
            case FSBlockError::FS_PTR_TO_UNKNOWN_BLOCK:
                ss << "This reference points to a block of unknown type"; break;
            case FSBlockError::FS_PTR_TO_EMPTY_BLOCK:
                ss << "This reference points to an empty block"; break;
            case FSBlockError::FS_PTR_TO_BOOT_BLOCK:
                ss << "This reference points to a boot block"; break;
            case FSBlockError::FS_PTR_TO_ROOT_BLOCK:
                ss << "This reference points to the root block"; break;
            case FSBlockError::FS_PTR_TO_BITMAP_BLOCK:
                ss << "This reference points to a bitmap block"; break;
            case FSBlockError::FS_PTR_TO_USERDIR_BLOCK:
                ss << "This reference points to a user directory block"; break;
            case FSBlockError::FS_PTR_TO_FILEHEADER_BLOCK:
                ss << "This reference points to a file header block"; break;
            case FSBlockError::FS_PTR_TO_FILELIST_BLOCK:
                ss << "This reference points to a file header block"; break;
            case FSBlockError::FS_PTR_TO_DATA_BLOCK:
                ss << "This reference points to a data block"; break;
            case FSBlockError::FS_EXPECTED_DATABLOCK_NR:
                ss << "Invalid data block position number"; break;
            case FSBlockError::FS_INVALID_HASHTABLE_SIZE:
                ss << "Expected $48 (72 hash table entries)"; break;

            default:
                ss << "???";
        }
    };

    for (isize i = 0; i < fs.traits.bsize / 4; i += 4) {

        optional<u32> expected;

        if (auto fault = xray32(node, i, strict, expected); fault != FSBlockError::FS_OK) {

            auto *data = node.data();
            auto type = fs.typeof(node.nr, i);

            ss << "+";
            hex(4, i, "  ");
            hex4(((u32 *)data)[i], "  ");

            ss << std::setw(36) << std::left << std::setfill(' ') << FSItemTypeEnum::help(type);
            describe(fault, expected);
            ss << std::endl;

            errors++;
        }
    }

    if (errors) {

        os << "Entry  Data         Item type                           Expected" << std::endl;
        os << string(80, '-') << std::endl;
        string line;
        while(std::getline(ss, line)) os << line << '\n';

    } else {

        os << "No errors found" << std::endl;
    }

    return errors;
}

FSBlockError
FSDoctor::checkBlockType(Block nr, FSBlockType type) const
{
    return checkBlockType(nr, type, type);
}

FSBlockError
FSDoctor::checkBlockType(Block nr, FSBlockType type, FSBlockType altType) const
{
    auto t = fs.typeof(nr);

    if (t != type && t != altType) {

        switch (t) {

            case FSBlockType::EMPTY:      return FSBlockError::FS_PTR_TO_EMPTY_BLOCK;
            case FSBlockType::BOOT:       return FSBlockError::FS_PTR_TO_BOOT_BLOCK;
            case FSBlockType::ROOT:       return FSBlockError::FS_PTR_TO_ROOT_BLOCK;
            case FSBlockType::BITMAP:     return FSBlockError::FS_PTR_TO_BITMAP_BLOCK;
            case FSBlockType::BITMAP_EXT: return FSBlockError::FS_PTR_TO_BITMAP_EXT_BLOCK;
            case FSBlockType::USERDIR:    return FSBlockError::FS_PTR_TO_USERDIR_BLOCK;
            case FSBlockType::FILEHEADER: return FSBlockError::FS_PTR_TO_FILEHEADER_BLOCK;
            case FSBlockType::FILELIST:   return FSBlockError::FS_PTR_TO_FILELIST_BLOCK;
            case FSBlockType::DATA_OFS:   return FSBlockError::FS_PTR_TO_DATA_BLOCK;
            case FSBlockType::DATA_FFS:   return FSBlockError::FS_PTR_TO_DATA_BLOCK;
            default:                      return FSBlockError::FS_PTR_TO_UNKNOWN_BLOCK;
        }
    }

    return FSBlockError::FS_OK;
}

}
