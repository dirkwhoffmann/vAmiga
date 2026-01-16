// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystems/CBM/FSBlock.h"
#include "FileSystems/CBM/FileSystem.h"
#include "utl/io.h"
#include "utl/support.h"
#include <algorithm>
#include <fstream>

namespace retro::vault::cbm {

FSBlock::FSBlock(FileSystem *ref, BlockNr nr) : fs(ref), cache(ref->cache)
{
    this->nr = nr;
}

FSBlock::FSBlock(FileSystem *ref, BlockNr nr, FSBlockType t) : FSBlock(ref, nr)
{
    init(t);
}

FSBlock::~FSBlock()
{

}

void
FSBlock::init(FSBlockType t)
{
    type = t;

    switch (type) {

        case FSBlockType::EMPTY:

            dataCache.dealloc();
            break;

        case FSBlockType::UNKNOWN:

            break;

        case FSBlockType::BAM:

            break;

        case FSBlockType::USERDIR:

            break;

        case FSBlockType::DATA:

            break;

        default:
            break;
    }
}

/*
FSBlock *
FSBlock::make(FileSystem *ref, BlockNr nr, FSBlockType type)
{
    switch (type) {

        case FSBlockType::EMPTY:
        case FSBlockType::USERDIR:
        case FSBlockType::DATA:

            return new FSBlock(ref, nr, type);

        default:
            throw FSError(FSError::FS_WRONG_BLOCK_TYPE);
    }
}
*/

std::vector<BlockNr>
FSBlock::refs(const std::vector<const FSBlock *> blocks)
{
    std::vector<BlockNr> result;
    for (auto &it : blocks) { if (it) result.push_back(it->nr); }
    return result;
}

/*
const char *
FSBlock::objectName() const
{
    switch (type) {

        case FSBlockType::UNKNOWN:     return "FSBlock (Unknown)";
        case FSBlockType::EMPTY:       return "FSBlock (Empty)";
        case FSBlockType::USERDIR:     return "FSBlock (UserDir)";
        case FSBlockType::DATA:        return "FSBlock (Data)";

        default:
            throw FSError(FSError::FS_WRONG_BLOCK_TYPE);
    }
}
*/

bool
FSBlock::is(FSBlockType type) const
{
    return this->type == type;
}

bool
FSBlock::isEmpty() const
{
    return type == FSBlockType::EMPTY;
}

bool
FSBlock::isBAM() const
{
    return type == FSBlockType::BAM;
}

/*
bool
FSBlock::isDirectory() const
{
    return type == FSBlockType::ROOT || type == FSBlockType::USERDIR;
}

bool
FSBlock::isRegular() const
{
    return isFile() || isDirectory();
}

bool
FSBlock::isData() const
{

    // Note: As FFS data blocks have no header, each block can be a data block.
    return true;
}
*/

fs::path
FSBlock::sanitizedPath() const
{
    return "";
    /*
    fs::path result;

    auto nodes = fs->collect(*this, [](auto *node) { return node->getParentDirBlock(); });

    // Delete the root node
    if (!nodes.empty()) nodes.pop_back();

    for (auto &it : nodes) {

        auto name = it->getName().path();
        result = result.empty() ? name : name / result;
    }

    return result;
    */
}

bool
FSBlock::matches(const FSPattern &pattern) const
{
    return false;
    /*
    if (pattern.isAbsolute()) {
        printf("Abs matching %s and %s (%d)\n", absName().c_str(), pattern.glob.c_str(), pattern.match(absName()));
        return pattern.match(absName());
    } else {
        printf("Rel matching %s and %s (%d)\n", relName().c_str(), pattern.glob.c_str(), pattern.match(relName()));
        return pattern.match(cppName());
    }
    */
}

isize
FSBlock::bsize() const
{
    return fs->traits.bsize;
}

isize
FSBlock::dsize() const
{
    switch (type) {

        case FSBlockType::DATA:     return bsize() - 24;
        case FSBlockType::EMPTY:    return bsize();

        default:
            fatalError;
    }
}

FSItemType
FSBlock::itemType(isize byte) const
{
    switch (type) {

        case FSBlockType::BAM:

            switch (byte) {

                case 0x00: return FSItemType::FIRST_DIR_TRACK;
                case 0x01: return FSItemType::FIRST_DIR_SECTOR;
                case 0x02: return FSItemType::DOS_VERSION;
                case 0xA2: return FSItemType::DISK_ID;
                case 0xA3: return FSItemType::DISK_ID;
                case 0xA5: return FSItemType::DOS_TYPE;
                case 0xA6: return FSItemType::DOS_TYPE;
            }
            if (byte >= 0x04 && byte <= 0x8F) return FSItemType::ALLOCATION_BITS;
            if (byte >= 0x90 && byte <= 0x9F) return FSItemType::DISK_NAME;

            return FSItemType::UNUSED;

        case FSBlockType::USERDIR:

            if (byte == 0) return FSItemType::TRACK_LINK;
            if (byte == 1) return FSItemType::SECTOR_LINK;

            byte &= 0x1F;

            switch (byte) {

                case 0x02: return FSItemType::FILE_TYPE;
                case 0x03: return FSItemType::FIRST_FILE_TRACK;
                case 0x04: return FSItemType::FIRST_FILE_SECTOR;
                case 0x15: return FSItemType::FIRST_REL_TRACK;
                case 0x16: return FSItemType::FIRST_REL_SECTOR;
                case 0x17: return FSItemType::REL_RECORD_LENGTH;
                case 0x1E: return FSItemType::FILE_LENGTH_LO;
                case 0x1F: return FSItemType::FILE_LENGTH_HI;
            }

            if (byte >= 0x05 && byte <= 0x14) return FSItemType::FILE_NAME;
            if (byte >= 0x18 && byte <= 0x1D) return FSItemType::GEOS;

            return FSItemType::UNUSED;

        case FSBlockType::DATA:

            if (byte == 0) return FSItemType::TRACK_LINK;
            if (byte == 1) return FSItemType::SECTOR_LINK;

            return FSItemType::DATA;

        default:
            fatalError;
    }
}

u32
FSBlock::typeID() const
{
    return type == FSBlockType::EMPTY ? 0 : get32(0);
}

u32
FSBlock::subtypeID() const
{
    return type == FSBlockType::EMPTY ? 0 : get32((bsize() / 4) - 1);
}

const u8 *
FSBlock::addr32(isize nr) const
{
    return (data() + 4 * nr) + (nr < 0 ? bsize() : 0);
}

u8 *
FSBlock::addr32(isize nr)
{
    return (data() + 4 * nr) + (nr < 0 ? bsize() : 0);
}

u8 *
FSBlock::data()
{
    if (dataCache.empty()) {

        dataCache.alloc(bsize());
        cache.dev.readBlock(dataCache.ptr, nr);
    }

    assert(dataCache.size == bsize());
    assert(dataCache.ptr);

    return dataCache.ptr;
}

const u8 *
FSBlock::data() const
{
    return const_cast<const u8 *>(const_cast<FSBlock *>(this)->data());
}

FSBlock &
FSBlock::mutate() const
{
    cache.markAsDirty(nr);
    return const_cast<FSBlock &>(*this);
}

void
FSBlock::invalidate()
{
    cache.markAsDirty(nr);
}

void
FSBlock::flush()
{
    if (!dataCache.empty()) {
        
        cache.dev.writeBlock(dataCache.ptr, nr);
    }
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
FSBlock::dumpInfo(std::ostream &os) const
{
    using namespace utl;

    /*
    auto byteStr = [&os](isize num) {

        auto str = std::to_string(num) + " Byte" + (num == 1 ? "" : "s");
        os << std::setw(13) << std::left << std::setfill(' ') << str;
    };
    auto blockStr = [&os](isize num) {

        auto str = std::to_string(num) + " Block" + (num == 1 ? "" : "s");
        os << std::setw(13) << std::left << std::setfill(' ') << str;
    };
    */
    /*
    switch (type) {

        case FSBlockType::FILEHEADER:
        {
            auto name = getName().str();
            auto size = getFileSize();
            auto listBlocks = isize(fs->collectListBlocks(nr).size());
            auto dataBlocks = isize(fs->collectDataBlocks(nr).size());
            auto totalBlocks = 1 + listBlocks + dataBlocks;
            auto tab = int(name.size()) + 4;

            os << std::setw(tab) << std::left << "Name";
            os << "Size         Header       Lists        Data         Total" << std::endl;

            os << std::setw(tab) << std::left << name;
            byteStr(size);
            blockStr(1);
            blockStr(listBlocks);
            blockStr(dataBlocks);
            blockStr(totalBlocks);
            os << std::endl;
        }
        default:
            break;
    }
    */
}

void
FSBlock::dumpBlocks(std::ostream &os) const
{
    using namespace utl;

    /*
    switch (type) {

        case FSBlockType::FILEHEADER:
        {
            auto size = getFileSize();
            auto listBlocks = fs->collectListBlocks(nr);
            auto dataBlocks = fs->collectDataBlocks(nr);
            auto totalBlocks = 1 + listBlocks.size() + dataBlocks.size();

            os << tab("Name");
            os << getName().str() << std::endl;
            os << tab("Blocks");
            os << totalBlocks << " Block" << (totalBlocks == 1 ? "" : "s") << std::endl;
            os << tab("Size");
            os << size << " Byte" << (size == 1 ? "" : "s") << std::endl;
            os << tab("File header block");
            os << nr << std::endl;
            os << tab("File list blocks");
            os << FSBlock::rangeString(listBlocks) << std::endl;
            os << tab("Data blocks");
            os << FSBlock::rangeString(dataBlocks) << std::endl;
        }
        default:
            break;
    }
    */
}

void
FSBlock::dumpStorage(std::ostream &os) const
{
    fs->doctor.dump(nr, os);
}

/*
void
FSBlock::hexDump(std::ostream &os, const DumpOpt &opt) const
{
    if (type == FSBlockType::EMPTY) {

        Dumpable::dump(os, [&](isize offset, isize bytes) { return offset < bsize() ? 0 : -1; }, opt, DumpFmt{} );

    } else {

        Dumpable::dump(os, Dumpable::dataProvider(data(), bsize()), opt, DumpFmt{});
    }
}
*/

string
FSBlock::rangeString(const std::vector<BlockNr> &vec)
{
    if (vec.empty()) return "";

    // Create a sorted copy
    std::vector<BlockNr> v = vec;
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

    return utl::concat(chunks, ", ");
}

void
FSBlock::importBlock(const u8 *src, isize size)
{    
    assert(src);
    assert(size == bsize());

    auto *bdata = data();

    if (bdata) {
        std::memcpy(data(), src, size);
    }
}

void
FSBlock::exportBlock(u8 *dst, isize size) const
{
    assert(dst);
    assert(size == bsize());

    auto *bdata = data();

    if (bdata) {
        std::memcpy(dst, data(), size);
    } else {
        std::memset(dst, 0, size);
    }
}

FSFault
FSBlock::exportBlock(const fs::path &path) const
{
    /*
     switch (type) {

     case FSBlockType::USERDIR:    return exportUserDirBlock(path);
     case FSBlockType::FILEHEADER: return exportFileHeaderBlock(path);

     default:
     return FSError::FS_OK;
     }
     */
    return FSError::FS_OK;
}

FSFault
FSBlock::exportUserDirBlock(const fs::path &path) const
{
    // Assemble the host file name
    auto filename = path / sanitizedPath();
    loginfo(FS_DEBUG >= 2, "Creating directory %s\n", filename.string().c_str());

    // Create directory
    if (!utl::createDirectory(filename)) return FSError::FS_CANNOT_CREATE_DIR;

    return FSError::FS_OK;
}

FSFault
FSBlock::exportFileHeaderBlock(const fs::path &path) const
{
    // Assemble the host file name
    auto filename = path; //  / sanitizedPath();
    loginfo(FS_DEBUG >= 2, "  Exporting file %s\n", filename.string().c_str());

    // Open file
    std::ofstream file(filename, std::ofstream::binary);
    if (!file.is_open()) return FSError::FS_CANNOT_CREATE_FILE;

    // Write data
    writeData(file);
    return FSError::FS_OK;
}

bool
FSBlock::hasName() const
{
    switch (type) {

        case FSBlockType::BAM:

            return true;

        default:

            return false;
    }
}

PETName<16>
FSBlock::getName() const
{
    switch (type) {

        case FSBlockType::BAM:

            return PETName<16>(data() + 0x90);

        default:
            return PETName<16>("");
    }
}

void
FSBlock::setName(PETName<16> name)
{
    switch (type) {

        case FSBlockType::BAM:

            name.write(data() + 0x90);

        default:
            break;
    }
}

isize
FSBlock::writeData(std::ostream &os) const
{
    Buffer<u8> buffer;
    
    // Write block into buffer
    extractData(buffer);
    
    // Export the buffer
    os << buffer;
    
    return buffer.size;
}

isize
FSBlock::writeData(std::ostream &os, isize size) const
{
    isize count = std::min(dsize(), size);
    auto *bdata = data();

    switch (type) {
            
        case FSBlockType::DATA:
            
            os.write((char *)(bdata + 2), count);
            return count;

        default:
            fatalError;
    }
}

isize
FSBlock::extractData(Buffer<u8> &buf) const
{
    // Only call this function for file header blocks
    // if (type != FSBlockType::FILEHEADER) throw FSError(FSError::FS_NOT_A_FILE);

    // isize bytesRemaining = getFileSize();
    isize bytesTotal = 0;

    /*
    buf.init(bytesRemaining);

    for (auto &it : fs->collectDataBlocks(this->nr)) {

        isize bytesWritten = it->writeData(buf, bytesTotal, bytesRemaining);
        bytesTotal += bytesWritten;
        bytesRemaining -= bytesWritten;
    }

    if (bytesRemaining != 0) {
        logwarn("%ld remaining bytes. Expected 0.\n", bytesRemaining);
    }
    */

    return bytesTotal;

    /*
     isize blocksTotal = 0;
    // Start here and iterate through all connected file list blocks
    const FSBlock *block = this;
    
    while (block && blocksTotal < fs->numBlocks()) {

        blocksTotal++;
        
        // Iterate through all data blocks references in this block
        isize num = std::min(block->getNumDataBlockRefs(), block->getMaxDataBlockRefs());
        for (isize i = 0; i < num; i++) {
            
            Block ref = block->getDataBlockRef(i);
            if (FSBlock *dataBlock = fs->dataBlockPtr(ref)) {

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
     */
}

isize
FSBlock::writeData(Buffer<u8> &buf, isize offset, isize count) const
{
    count = std::min(dsize(), count);
    auto *bdata = data();

    switch (type) {
            
        case FSBlockType::DATA:
            
            std::memcpy((void *)(buf.ptr + offset), (void *)(bdata + 22), count);
            return count;

        case FSBlockType::EMPTY:

            std::memset((void *)(buf.ptr + offset), 0, count);
            return count;

        default:
            fatalError;
    }
}

isize
FSBlock::overwriteData(Buffer<u8> &buf)
{
    /*
    // Only call this function for file header blocks
    assert(type == FSBlockType::FILEHEADER);
    
    isize bytesRemaining = getFileSize();
    isize bytesTotal = 0;
    isize blocksTotal = 0;
    
    assert(buf.size == bytesRemaining);
    
    // Start here and iterate through all connected file list blocks
    const FSBlock *block = this;

    while (block && blocksTotal < fs->blocks()) {

        blocksTotal++;
        
        // Iterate through all data blocks references in this block
        isize num = std::min(block->getNumDataBlockRefs(), block->getMaxDataBlockRefs());
        for (isize i = 0; i < num; i++) {

            BlockNr ref = block->getDataBlockRef(i);
            if (auto &dataBlock = fs->fetch(ref); dataBlock.isData()) { //} dataBlockPtr(ref)) {

                isize bytesWritten = dataBlock.mutate().overwriteData(buf, bytesTotal, bytesRemaining);
                bytesTotal += bytesWritten;
                bytesRemaining -= bytesWritten;
                
            } else {
                
                logwarn("Ignoring block %ld (no data block)\n", ref);
            }
        }
        
        // Continue with the next list block
        block = block->getNextListBlock();
    }
    
    if (bytesRemaining != 0) {
        logwarn("%ld remaining bytes. Expected 0.\n", bytesRemaining);
    }
    
    return bytesTotal;
    */
    return 0;
}

isize
FSBlock::overwriteData(Buffer<u8> &buf, isize offset, isize count)
{
    /*
    count = std::min(dsize(), count);
    auto *bdata = data();

    switch (type) {
            
        case FSBlockType::DATA:
            
            std::memcpy((void *)(bdata + 2), (void *)(buf.ptr + offset), count);
            return count;
                        
        default:
            fatalError;
    }
    */
    return 0;
}

}
