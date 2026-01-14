// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystems/CBM/FSDoctor.h"
#include "FileSystems/CBM/FileSystem.h"
#include "utl/io.h"
#include "utl/support.h"
#include <unordered_map>
#include <unordered_set>
#include <sstream>

//
// Macros used inside the check() methods
//

#define EXPECT_VALUE(exp) { \
if ((u32)value != u32(exp)) { expected = u32(exp); return FSBlockError::EXPECTED_VALUE; } }

#define EXPECT_CHECKSUM EXPECT_VALUE(node.checksum())

#define EXPECT_LESS_OR_EQUAL(exp) { \
if ((u32)value > (u32)exp) \
{ expected = (u8)(exp); return FSBlockError::EXPECTED_SMALLER_VALUE; } }

#define EXPECT_REF { \
if (!fs.block(value)) return FSBlockError::EXPECTED_REF; }

#define EXPECT_SELFREF { \
if ((u32)value != (u32)ref) { expected = ref; return FSBlockError::EXPECTED_SELFREF; } }

#define EXPECT_FILEHEADER_REF { \
if (!fs.is(value, FSBlockType::FILEHEADER)) { \
return FSBlockError::EXPECTED_FILE_HEADER_BLOCK; } }

#define EXPECT_HASH_REF { \
if (!fs.is(value, FSBlockType::FILEHEADER) && !fs.is(value, FSBlockType::USERDIR)) { \
return FSBlockError::EXPECTED_HASHABLE_BLOCK; } }

#define EXPECT_OPTIONAL_HASH_REF { \
if (value) { EXPECT_HASH_REF } }

#define EXPECT_PARENT_DIR_REF { \
if (!fs.is(value, FSBlockType::ROOT) && !fs.is(value, FSBlockType::USERDIR)) { \
return FSBlockError::EXPECTED_USERDIR_OR_ROOT; } }

#define EXPECT_FILELIST_REF { \
if (!fs.is(value, FSBlockType::FILELIST)) { \
return FSBlockError::EXPECTED_FILE_LIST_BLOCK; } }

#define EXPECT_OPTIONAL_FILELIST_REF { \
if (value) { EXPECT_FILELIST_REF } }

#define EXPECT_BITMAP_REF(nr) { \
if (!fs.is(value, FSBlockType::BITMAP)) { \
if (fs.getBmBlocks().size() > usize(nr)) { expected = fs.getBmBlocks()[nr]; } \
return FSBlockError::EXPECTED_BITMAP_BLOCK; } }

#define EXPECT_OPTIONAL_BITMAP_REF(nr) { \
if (value) { EXPECT_BITMAP_REF(nr) } }

#define EXPECT_BITMAP_EXT_REF { \
if (!fs.is(value, FSBlockType::BITMAP_EXT)) { \
return FSBlockError::EXPECTED_BITMAP_EXT_BLOCK; } }

#define EXPECT_OPTIONAL_BITMAP_EXT_REF { \
if (value) { EXPECT_BITMAP_EXT_REF } }

#define EXPECT_DATABLOCK_REF { \
if (traits.ofs() && !fs.is(value, FSBlockType::DATA_OFS)) { \
return FSBlockError::EXPECTED_DATA_BLOCK; } }

#define EXPECT_OPTIONAL_DATABLOCK_REF { \
if (value) { EXPECT_DATABLOCK_REF } }

#define EXPECT_DATABLOCK_NUMBER { \
if (value == 0) return FSBlockError::EXPECTED_DATABLOCK_NR; }

#define EXPECT_HTABLE_SIZE { \
if (isize(value) != (traits.bsize / 4) - 56) { \
expected = u32((traits.bsize / 4) - 56); return FSBlockError::INVALID_HASHTABLE_SIZE; } }

namespace retro::vault::cbm {

FSDoctor::FSDoctor(FileSystem& fs, FSAllocator &a) : FSService(fs), allocator(a)
{

}

void
FSDoctor::dump(BlockNr nr, std::ostream &os)
{
    using namespace utl;

    auto &p = fs.fetch(nr);

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

        case FSBlockType::BAM:

            os << tab("Name");
            os << p.getName().str() << std::endl;
            break;

        case FSBlockType::USERDIR:

            os << tab("Name");
            os << p.getName().str() << std::endl;
            os << tab("Next");
            os << dec(p.getNextHashRef()) << std::endl;
            break;

        case FSBlockType::DATA:

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
}

isize
FSDoctor::xray(bool strict)
{
    std::stringstream ss;
    return xray(strict, ss, false);
}

isize
FSDoctor::xray(bool strict, std::ostream &os, bool verbose)
{
    diagnosis.blockErrors = {};

    for (BlockNr nr = 0; isize(nr) < traits.blocks; nr++) {

        if (auto errors = xray(nr, strict)) {

            if (verbose) {

                if (!diagnosis.blockErrors.empty()) os << std::endl;
                xray(nr, strict, os);

            } else {

                os << utl::tab("Block " + std::to_string(nr) + "");
                os << errors << (errors == 1 ? " anomaly" : " anomalies") << std::endl;
            }

            diagnosis.blockErrors.push_back(BlockNr(nr));
        }
    }

    return isize(diagnosis.blockErrors.size());
}

isize
FSDoctor::xrayBitmap(bool strict)
{
    std::unordered_set<BlockNr> used;

    /*
    // Extract the directory tree
    auto tree = fs.build(fs.root(), { .depth = MAX_ISIZE });

    // Collect all used blocks
    for (auto &it : tree.dfs()) {

        used.insert(it.nr);
        auto &node = fs.fetch(it.nr);

        if (node.isFile()) {

            auto listBlocks = fs.collectListBlocks(it.nr);
            auto dataBlocks = fs.collectDataBlocks(it.nr);
            used.insert(listBlocks.begin(), listBlocks.end());
            used.insert(dataBlocks.begin(), dataBlocks.end());
        }
    }
    used.insert(fs.getBmBlocks().begin(), fs.getBmBlocks().end());
    used.insert(fs.getBmExtBlocks().begin(), fs.getBmExtBlocks().end());

    // Check all blocks (ignoring the first two boot blocks)
    for (isize i = 2, capacity = fs.blocks(); i < capacity; i++) {

        bool allocated = allocator.isAllocated(BlockNr(i));
        bool contained = used.contains(BlockNr(i));

        if (allocated && !contained) {
            diagnosis.unusedButAllocated.push_back(BlockNr(i));
        } else if (!allocated && contained) {
            diagnosis.usedButUnallocated.push_back(BlockNr(i));
        }
    }

    return
    (isize)diagnosis.unusedButAllocated.size() +
    (isize)diagnosis.usedButUnallocated.size();
    */
    return 0;
}

isize
FSDoctor::xrayBitmap(std::ostream &os, bool strict)
{
    auto result = xrayBitmap(strict);

    auto &usedButUnallocated = fs.doctor.diagnosis.usedButUnallocated;
    auto &unusedButAllocated = fs.doctor.diagnosis.unusedButAllocated;

    auto blocks = [&](size_t s) { return std::to_string(s) + (s == 1 ? " block" : " blocks"); };

    if (auto total = usedButUnallocated.size() + unusedButAllocated.size(); total) {

        os << utl::tab("Bitmap anomalies:") << blocks(total) << std::endl;

        if (!usedButUnallocated.empty()) {

            os << utl::tab("Used but unallocated:");
            os << FSBlock::rangeString(usedButUnallocated) << std::endl;
        }
        if (!unusedButAllocated.empty()) {

            os << utl::tab("Allocated but unused:");
            os << FSBlock::rangeString(unusedButAllocated) << std::endl;
        }
    }
    return result;
}

isize
FSDoctor::xray(BlockNr ref, bool strict) const
{
    auto &node = fs.fetch(ref);
    isize count = 0;

    for (isize i = 0; i < node.bsize(); i += 4) {

        std::optional<u32> expected;
        if (auto error = xray32(ref, i, strict, expected); error != FSBlockError::OK) {

            count++;
            loginfo(FS_DEBUG, "Block %ld [%ld]: %s\n", node.nr, i, FSBlockErrorEnum::key(error));
        }
    }

    return count;
}

FSBlockError
FSDoctor::xray8(BlockNr ref, isize pos, bool strict, optional<u8> &expected) const
{
    // auto &node = fs.fetch(ref);
    optional<u32> exp;
    auto result = xray32(ref, pos & ~3, strict, exp);
    if (exp) expected = GET_BYTE(*exp, 3 - (pos & 3));
    return result;
}

FSBlockError
FSDoctor::xray32(BlockNr ref, isize pos, bool strict, optional<u32> &expected) const
{
    return FSBlockError::OK;
}

isize
FSDoctor::xray(BlockNr ref, bool strict, std::ostream &os) const
{
    return 0;
}

void
FSDoctor::rectify(bool strict)
{
    auto *mfs = dynamic_cast<FileSystem *>(&fs);
    if (!mfs) throw FSError(FSError::FS_READ_ONLY);

    xray(strict);

    // Rectify all erroneous blocks
    for (auto &it : diagnosis.blockErrors) rectify(it, strict);
}

void
FSDoctor::rectify(BlockNr ref, bool strict)
{
    auto &node = fs.fetch(ref);

    for (isize i = 0; i < traits.bsize / 4; i += 4) {

        optional<u32> expected;

        if (auto fault = xray32(ref, i, strict, expected); fault != FSBlockError::OK) {

            if (expected) {

                auto &mutatableNode = node.mutate();
                auto *data = mutatableNode.data();
                mutatableNode.mutate().write32(data + i, *expected);
            }
        }
    }
}

void
FSDoctor::rectifyBitmap(bool strict)
{
    xrayBitmap(strict);

    for (auto &it : diagnosis.unusedButAllocated) {
        allocator.markAsFree(BlockNr(it));
    }
    for (auto &it : diagnosis.usedButUnallocated) {
        allocator.markAsAllocated(BlockNr(it));
    }
}

string
FSDoctor::ascii(BlockNr nr, isize offset, isize len) const noexcept
{
    assert(offset + len <= traits.bsize);

    return  utl::createAscii(fs.fetch(nr).data() + offset, len);
}

void
FSDoctor::createUsageMap(u8 *buffer, isize len) const
{
    // Setup priorities
    i8 pri[12];
    pri[isize(FSBlockType::UNKNOWN)]      = 0;
    pri[isize(FSBlockType::EMPTY)]        = 1;
    pri[isize(FSBlockType::BAM)]          = 4;
    pri[isize(FSBlockType::USERDIR)]      = 3;
    pri[isize(FSBlockType::DATA)]         = 2;

    isize max = traits.blocks;

    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = (u8)FSBlockType::UNKNOWN;

    // Mark all free blocks
    for (isize i = 0; i < max; i++) buffer[i * (len - 1) / (max - 1)] = (u8)FSBlockType::EMPTY;

    // Mark all used blocks
    for (isize i = 0; i < max; i++) {

        if (auto type = fs.typeOf(BlockNr(i)); type != FSBlockType::EMPTY) {

            auto val = u8(type);
            auto pos = i * (len - 1) / (max - 1);
            if (pri[buffer[pos]] < pri[val]) buffer[pos] = val;
            if (pri[buffer[pos]] == pri[val] && pos > 0 && buffer[pos-1] != val) buffer[pos] = val;
        }
    }

    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        if (buffer[pos] == (u8)FSBlockType::UNKNOWN) buffer[pos] = buffer[pos - 1];
    }
}

void
FSDoctor::createAllocationMap(u8 *buffer, isize len) const
{
    auto &unusedButAllocated = diagnosis.unusedButAllocated;
    auto &usedButUnallocated = diagnosis.usedButUnallocated;

    isize max = traits.blocks;

    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = 255;

    // Mark all free blocks
    for (isize i = 0; i < max; i++) buffer[i * (len - 1) / (max - 1)] = 0;

    // Mark all used blocks
    for (isize i = 0; i < max; i++) {

        if (auto type = fs.typeOf(BlockNr(i)); type != FSBlockType::EMPTY) {
            buffer[i * (len - 1) / (max - 1)] = 1;
        }
    }

    // Mark all erroneous blocks
    for (auto &it : unusedButAllocated) buffer[it * (len - 1) / (max - 1)] = 2;
    for (auto &it : usedButUnallocated) buffer[it * (len - 1) / (max - 1)] = 3;

    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        if (buffer[pos] == 255) buffer[pos] = buffer[pos - 1];
    }
}

void
FSDoctor::createHealthMap(u8 *buffer, isize len) const
{
    auto &blockErrors = diagnosis.blockErrors;

    isize max = traits.blocks;

    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = 255;

    // Mark all free blocks
    for (isize i = 0; i < max; i++) buffer[i * (len - 1) / (max - 1)] = 0;

    // Mark all used blocks
    for (isize i = 0; i < max; i++) {

        if (auto type = fs.typeOf(BlockNr(i)); type != FSBlockType::EMPTY) {
            buffer[i * (len - 1) / (max - 1)] = 1;
        }
    }

    // Mark all erroneous blocks
    for (auto &it : blockErrors) buffer[it * (len - 1) / (max - 1)] = 2;

    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        if (buffer[pos] == 255) buffer[pos] = buffer[pos - 1];
    }
}

isize
FSDoctor::nextBlockOfType(FSBlockType type, BlockNr after) const
{
    assert(isize(after) < traits.blocks);

    isize result = after;

    do {
        result = (result + 1) % traits.blocks;
        if (fs.typeOf(BlockNr(result)) == type) return result;

    } while (result != isize(after));

    return -1;
}

}
