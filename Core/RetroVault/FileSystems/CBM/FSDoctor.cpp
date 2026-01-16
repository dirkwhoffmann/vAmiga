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

#define EXPECT_BYTE(exp) { \
if (value != (exp)) { expected = (u8)(exp); return FSBlockError::EXPECTED_VALUE; } }

#define EXPECT_MIN(min) { \
if (value < (min)) { expected = (u8)(min); return FSBlockError::EXPECTED_LARGER_VALUE; } }

#define EXPECT_MAX(max) { \
if (value > (max)) { expected = (u8)(max); return FSBlockError::EXPECTED_SMALLER_VALUE; } }

#define EXPECT_RANGE(min,max) { \
EXPECT_MIN(min); EXPECT_MAX(max) }

#define EXPECT_TRACK_REF(s) \
EXPECT_RANGE(0, traits.numTracks() + 1)

#define EXPECT_SECTOR_REF(t) { \
if (isize num = traits.numSectors(t)) \
EXPECT_RANGE(0,num) else if (strict) EXPECT_MAX(254) }


namespace retro::vault::cbm {

FSDoctor::FSDoctor(FileSystem& fs, FSAllocator &a) : FSService(fs), allocator(a)
{

}

void
FSDoctor::dump(BlockNr nr, std::ostream &os)
{
    using namespace utl;

    auto &p   = fs.fetch(nr);
    auto ts   = p.tsLink();
    auto data = p.data();

    os << tab("Block");
    os << dec(nr) << std::endl;
    os << tab("Type");
    os << FSBlockTypeEnum::key(p.type) << std::endl;
    os << tab("TS link");
    os << ts.t << ":" << ts.s << std::endl;

    switch (p.type) {

        case FSBlockType::BAM:
        {
            os << tab("Name");
            os << p.getName().str() << std::endl;
            os << tab("DOS version");
            os << data[0x02] << std::endl;
            os << tab("DOS type");
            os << data[0xA5] << data[0xA6] << std::endl;
            break;
        }
        case FSBlockType::USERDIR:
        {
            isize slot = 0;
            for (const auto &it : fs.readDirBlock(nr)) {

                os << tab("Slot " + std::to_string(++slot));
                if (it.empty()) {
                    os << "<empty>" << std::endl;
                } else {
                    os << it.getName().str() << std::endl;
                }
                os << tab("Size");
                os << (it.fileSizeHi << 8 | it.fileSizeLo) << std::endl;
                os << tab("First data block");
                os << it.firstDataTrack << ":" << it.firstDataSector << std::endl;

                if (slot < 8) os << std::endl;
            }
            break;
        }
        case FSBlockType::DATA:
        {
            auto lastBlock = ts.t == 0;
            os << tab("Stored bytes");
            os << (lastBlock ? ts.s : 254) << std::endl;
            break;
        }
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

    // Read directory
    auto dir = fs.readDir();

    // Collect all used blocks
    for (auto &it : dir) {

        auto dataBlocks = fs.collectDataBlocks(it);
        used.insert(dataBlocks.begin(), dataBlocks.end());
    }

    // Read allocation map
    auto alloc = fs.readBitmap();

    // Check all blocks
    for (isize i = 0; i < fs.blocks(); ++i) {

        bool allocated = alloc[i];
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

    for (isize i = 0; i < node.bsize(); ++i) {

        std::optional<u8> expected;
        if (auto error = xray8(ref, i, strict, expected); error != FSBlockError::OK) {

            count++;
            loginfo(FS_DEBUG, "Block %ld [%ld]: %s\n", node.nr, i, FSBlockErrorEnum::key(error));
        }
    }

    return count;
}

FSBlockError
FSDoctor::xray8(BlockNr ref, isize pos, bool strict, optional<u8> &expected) const
{
    assert(pos >= 0 && pos < 256);

    auto& block = fs.fetch(ref);
    auto *data  = block.data();
    u8 value    = data[pos];

    switch (block.type) {

        case FSBlockType::BAM:

            switch (pos) {

                case 0x00: EXPECT_BYTE(18);               break;
                case 0x01: EXPECT_BYTE(1);                break;
                case 0x02: EXPECT_BYTE(0x41);             break;
                case 0xA0:
                case 0xA1:
                case 0xA4: if (strict) EXPECT_BYTE(0xA0); break;
                case 0xA5: EXPECT_BYTE('2');              break;
                case 0xA6: EXPECT_BYTE('A');              break;
                case 0xA7:
                case 0xA8:
                case 0xA9:
                case 0xAA: if (strict) EXPECT_BYTE(0xA0); break;
            }

            if (strict && pos >= 0xAB && pos <= 0xFF) EXPECT_BYTE(0x00);

            return FSBlockError::OK;

        case FSBlockType::USERDIR:

            if (pos == 0) EXPECT_TRACK_REF (data[pos + 1]);
            if (pos == 1) EXPECT_SECTOR_REF(data[pos - 1]);

            if (!utl::isZero(data + pos, 0x20)) {

                switch (pos & 0x1F) {

                    case 0x03: EXPECT_TRACK_REF (data[pos + 1]); break;
                    case 0x04: EXPECT_SECTOR_REF(data[pos - 1]); break;
                    case 0x15: EXPECT_TRACK_REF (data[pos + 1]); break;
                    case 0x16: EXPECT_SECTOR_REF(data[pos - 1]); break;
                    case 0x17: EXPECT_MAX(254);                  break;
                }
            }

            return FSBlockError::OK;

        case FSBlockType::DATA:

            if (pos == 0 && strict) EXPECT_TRACK_REF (data[pos + 1]);
            if (pos == 1 && strict) EXPECT_SECTOR_REF(data[pos - 1]);

            return FSBlockError::OK;

        default:
            fatalError;
    }

}

isize
FSDoctor::xray(BlockNr ref, bool strict, std::ostream &os) const
{
    auto &node   = fs.fetch(ref);
    auto errors  = isize(0);

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

        if (value) { hex4(*value); return; }

        switch (fault) {

            case FSBlockError::EXPECTED_VALUE:
                ss << "Value"; break;
            case FSBlockError::EXPECTED_SMALLER_VALUE:
                ss << "Smaller value"; break;
            case FSBlockError::EXPECTED_LARGER_VALUE:
                ss << "Larger value"; break;
            default:
                ss << "???";
        }
    };

    for (isize i = 0; i < traits.bsize; ++i) {

        optional<u8> expected;

        if (auto fault = xray8(ref, i, strict, expected); fault != FSBlockError::OK) {

            auto *data = node.data();
            auto type = fs.typeOf(node.nr, i);

            ss << std::setw(7) << std::left << std::to_string(node.nr);
            ss << "+";
            hex(4, i, "  ");
            hex4(node.read32(data + i), "  ");

            ss << std::setw(36) << std::left << std::setfill(' ') << FSItemTypeEnum::help(type);
            describe(fault, expected);
            ss << std::endl;

            errors++;
        }
    }

    if (errors) {

        os << "Block  Entry  Data         Item type                           Expected" << std::endl;
        string line;
        while(std::getline(ss, line)) os << line << '\n';

    } else {

        // os << "No errors found" << std::endl;
    }

    return errors;
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

    for (isize i = 0; i < fs.blocks(); ++i) {

        optional<u8> expected;

        if (auto fault = xray8(ref, i, strict, expected); fault != FSBlockError::OK) {

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

    // TODO: SETUP VECTOR, WRITE BACK

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
