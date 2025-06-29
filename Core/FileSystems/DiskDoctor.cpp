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
if (Fault e = fs.checkBlockType(value, FSBlockType::FILEHEADER_BLOCK); e != Fault::OK) return e; }

#define EXPECT_HASH_REF { \
if (Fault e = fs.checkBlockType(value, FSBlockType::FILEHEADER_BLOCK, FSBlockType::USERDIR_BLOCK); e != Fault::OK) return e; }

#define EXPECT_OPTIONAL_HASH_REF { \
if (value) { EXPECT_HASH_REF } }

#define EXPECT_PARENT_DIR_REF { \
if (Fault e = fs.checkBlockType(value, FSBlockType::ROOT_BLOCK, FSBlockType::USERDIR_BLOCK); e != Fault::OK) return e; }

#define EXPECT_FILELIST_REF { \
if (Fault e = fs.checkBlockType(value, FSBlockType::FILELIST_BLOCK); e != Fault::OK) return e; }

#define EXPECT_OPTIONAL_FILELIST_REF { \
if (value) { EXPECT_FILELIST_REF } }

#define EXPECT_BITMAP_REF { \
if (Fault e = fs.checkBlockType(value, FSBlockType::BITMAP_BLOCK); e != Fault::OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_REF { \
if (value) { EXPECT_BITMAP_REF } }

#define EXPECT_BITMAP_EXT_REF { \
if (Fault e = fs.checkBlockType(value, FSBlockType::BITMAP_EXT_BLOCK); e != Fault::OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_EXT_REF { \
if (value) { EXPECT_BITMAP_EXT_REF } }

#define EXPECT_DATABLOCK_REF { \
if (Fault e = fs.checkBlockType(value, FSBlockType::DATA_BLOCK_OFS, FSBlockType::DATA_BLOCK_FFS); e != Fault::OK) return e; }

#define EXPECT_OPTIONAL_DATABLOCK_REF { \
if (value) { EXPECT_DATABLOCK_REF } }

#define EXPECT_DATABLOCK_NUMBER { \
if (value == 0) return Fault::FS_EXPECTED_DATABLOCK_NR; }

#define EXPECT_HASHTABLE_SIZE { \
if (value != 72) return Fault::FS_INVALID_HASHTABLE_SIZE; }

namespace vamiga {

std::vector<Block>
DiskDoctor::xray(bool strict) const
{
    std::vector<Block> result;

    for (isize i = 0, capacity = fs.numBlocks(); i < capacity; i++) {

        if (xray(Block(i), strict)) result.push_back(Block(i));
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

}
