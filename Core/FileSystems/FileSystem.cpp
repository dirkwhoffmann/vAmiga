// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IOUtils.h"
#include "MutableFileSystem.h"
#include "MemUtils.h"

#include <climits>
#include <unordered_set>
#include <stack>
#include <algorithm>

namespace vamiga {

bool
FSTraits::adf() const
{
    auto size = blocks * bsize;
    
    return
    size == ADFFile::ADFSIZE_35_DD ||
    size == ADFFile::ADFSIZE_35_DD_81 ||
    size == ADFFile::ADFSIZE_35_DD_82 ||
    size == ADFFile::ADFSIZE_35_DD_83 ||
    size == ADFFile::ADFSIZE_35_DD_84 ||
    size == ADFFile::ADFSIZE_35_HD;
}

FileSystem::~FileSystem()
{

}

void
FileSystem::init(const MediaFile &file, isize part) throws
{
    switch (file.type()) {

        case FileType::ADF:  init(dynamic_cast<const ADFFile &>(file)); break;
        case FileType::HDF:  init(dynamic_cast<const HDFFile &>(file), part); break;

        default:
            throw AppError(Fault::FILE_TYPE_UNSUPPORTED);
    }
}

void
FileSystem::init(const ADFFile &adf)
{
    // Get a file system descriptor
    auto descriptor = adf.getFileSystemDescriptor();

    // Import the file system
    init(descriptor, adf.data.ptr, descriptor.numBlocks * 512);
}

void
FileSystem::init(const HDFFile &hdf, isize part)
{
    // Get a file system descriptor
    auto descriptor = hdf.getFileSystemDescriptor(part);

    // Import the file system
    assert(hdf.partitionSize(part) == descriptor.numBlocks * 512);
    init(descriptor, hdf.partitionData(part), hdf.partitionSize(part));
}

void
FileSystem::init(const FloppyDrive &dfn)
{
    // Convert the floppy drive into an ADF
    auto adf = ADFFile(dfn);

    // Initialize with the ADF
    init(adf);
}

void
FileSystem::init(const HardDrive &hdn, isize part)
{
    // Convert the hard drive into an HDF
    auto hdf = HDFFile(hdn);

    // Initialize with the HDF
    init(hdf, part);
}

void
FileSystem::init(const FSDescriptor &layout, u8 *buf, isize len)
{
    assert(buf);

    debug(FS_DEBUG, "Importing %ld blocks from buffer...\n", layout.numBlocks);

    // Check the consistency of the file system descriptor
    layout.checkCompatibility();

    // Only proceed if the volume is formatted
    if (layout.dos == FSFormat::NODOS) throw AppError(Fault::FS_UNFORMATTED);

    // Copy layout parameters
    traits.dos      = layout.dos;
    traits.blocks   = layout.numBlocks;
    traits.bytes    = layout.numBlocks * layout.bsize;
    traits.bsize    = layout.bsize;
    traits.reserved = layout.numReserved;
    rootBlock       = layout.rootBlock;
    bmBlocks        = layout.bmBlocks;
    bmExtBlocks     = layout.bmExtBlocks;

    // Create all blocks
    storage.init(layout.numBlocks);

    for (isize i = 0; i < layout.numBlocks; i++) {

        const u8 *data = buf + i * traits.bsize;
        if (auto type = predictType((Block)i, data); type != FSBlockType::EMPTY) {

            // Create new block
            storage[i].init(type);

            // Import block data
            storage[i].importBlock(data, traits.bsize);

            // Emulate errors
            /*
            auto *data = storage[i].data();
            for (isize i = 0; i < 20; i++) {
                data[rand() % 512] = rand() & 0xFF;
            }
            */
        }
    }

    // Set the current directory to '/'
    current = rootBlock;

    debug(FS_DEBUG, "Success\n");
}

bool
FileSystem::isInitialized() const noexcept
{
    return numBlocks() > 0;
}

bool
FileSystem::isFormatted() const noexcept
{
    // Check if the file system is initialized
    if (!isInitialized()) return false;

    // Check the DOS type
    if (traits.dos == FSFormat::NODOS) return false;

    // Check if the root block is present
    if (!storage.read(rootBlock, FSBlockType::ROOT)) return false;

    return true;
}

void
FileSystem::_dump(Category category, std::ostream &os) const noexcept
{
    using namespace util;

    switch (category) {

        case Category::Info:

            os << "Type   Size             Used    Free    Full  Name" << std::endl;
            [[fallthrough]];

        case Category::State:
        {
            auto info = getInfo();
            auto size = std::to_string(info.numBlocks) + " (x " + std::to_string(traits.bsize) + ")";

            if (isFormatted()) {

                os << std::setw(5) << std::left << ("DOS" + std::to_string(isize(traits.dos)));
                os << "  ";
                os << std::setw(15) << std::left << std::setfill(' ') << size;
                os << "  ";
                os << std::setw(6) << std::left << std::setfill(' ') << info.usedBlocks;
                os << "  ";
                os << std::setw(6) << std::left << std::setfill(' ') << info.freeBlocks;
                os << "  ";
                os << std::setw(3) << std::right << std::setfill(' ') << isize(info.fillLevel);
                os << "%  ";
                os << getName().c_str() << std::endl;

            } else {

                os << std::setw(5) << std::left << "NODOS";
                os << "  ";
                os << std::setw(15) << std::left << std::setfill(' ') << size;
                os << "  ";
                os << std::setw(6) << std::left << std::setfill(' ') << "--";
                os << "  ";
                os << std::setw(6) << std::left << std::setfill(' ') << "--";
                os << "  ";
                os << std::setw(3) << std::left << std::setfill(' ') << "--";
                os << "   ";
                os << "--" << std::endl;
            }
            break;
        }
        case Category::Properties:
        {
            auto info = getInfo();

            os << tab("Name");
            os << getName().cpp_str() << std::endl;
            os << tab("Created");
            os << getCreationDate() << std::endl;
            os << tab("Modified");
            os << getModificationDate() << std::endl;
            os << tab("Boot block");
            os << getBootBlockName() << std::endl;
            os << tab("Capacity");
            os << util::byteCountAsString(info.numBlocks * traits.bsize) << std::endl;
            os << tab("Block size");
            os << dec(traits.bsize) << " Bytes" << std::endl;
            os << tab("Blocks");
            os << dec(info.numBlocks) << std::endl;
            os << tab("Used");
            os << dec(info.usedBlocks);
            os << tab("Free");
            os << dec(info.freeBlocks);
            os << " (" <<  std::fixed << std::setprecision(2) << info.fillLevel << "%)" << std::endl;
            os << tab("Root block");
            os << dec(rootBlock) << std::endl;
            os << tab("Bitmap blocks");
            for (auto& it : bmBlocks) { os << dec(it) << " "; }
            os << std::endl;
            os << util::tab("Extension blocks");
            for (auto& it : bmExtBlocks) { os << dec(it) << " "; }
            os << std::endl;
            break;
        }
        case Category::Blocks:

            storage.dump(Category::Blocks, os);
            break;

        default:
            break;
    }
}

void
FileSystem::cacheInfo(FSInfo &result) const noexcept
{
    result.name = getName().cpp_str();
    result.creationDate = getCreationDate();
    result.modificationDate = getModificationDate();

    result.numBlocks = storage.numBlocks();
    result.freeBlocks = numUnallocated();
    result.usedBlocks = result.numBlocks - result.freeBlocks;
    result.freeBytes = result.freeBlocks * traits.bsize;
    result.usedBytes = result.usedBlocks * traits.bsize;
    result.fillLevel = double(100) * result.usedBlocks / result.numBlocks;
}

void
FileSystem::cacheStats(FSStats &result) const noexcept
{

}

FSName
FileSystem::getName() const noexcept
{
    auto *rb = storage.read(rootBlock, FSBlockType::ROOT);
    return rb ? rb->getName() : FSName("");
}

string
FileSystem::getCreationDate() const noexcept
{
    auto *rb = storage.read(rootBlock, FSBlockType::ROOT);
    return rb ? rb->getCreationDate().str() : "";
}

string
FileSystem::getModificationDate() const noexcept
{
    auto *rb = storage.read(rootBlock, FSBlockType::ROOT);
    // FSBlock *rb = rootBlockPtr(rootBlock);
    return rb ? rb->getModificationDate().str() : "";
}

string
FileSystem::getBootBlockName() const noexcept
{
    return BootBlockImage(storage[0].data(), storage[1].data()).name;
}

BootBlockType
FileSystem::bootBlockType() const noexcept
{
    return BootBlockImage(storage[0].data(), storage[1].data()).type;
}

FSBlock *
FileSystem::read(Block nr) noexcept
{
    stats.blockReads++;
    return storage.read(nr);
}

FSBlock *
FileSystem::read(Block nr, FSBlockType type) noexcept
{
    stats.blockReads++;
    return storage.read(nr, type);
}

FSBlock *
FileSystem::read(Block nr, std::vector<FSBlockType> types) noexcept
{
    stats.blockReads++;
    return storage.read(nr, types);
}

const FSBlock *
FileSystem::read(Block nr) const noexcept
{
    stats.blockReads++;
    return storage.read(nr);
}

const FSBlock *
FileSystem::read(Block nr, FSBlockType type) const noexcept
{
    stats.blockReads++;
    return storage.read(nr, type);
}

const FSBlock *
FileSystem::read(Block nr, std::vector<FSBlockType> types) const noexcept
{
    stats.blockReads++;
    return storage.read(nr, types);
}

FSBlock &
FileSystem::at(Block nr)
{
    stats.blockReads++;
    return storage.at(nr);
}

FSBlock &
FileSystem::at(Block nr, FSBlockType type)
{
    stats.blockReads++;
    return storage.at(nr, type);
}

FSBlock &
FileSystem::at(Block nr, std::vector<FSBlockType> types)
{
    stats.blockReads++;
    return storage.at(nr, types);
}

const FSBlock &
FileSystem::at(Block nr) const
{
    stats.blockReads++;
    return storage.at(nr);
}

const FSBlock &
FileSystem::at(Block nr, FSBlockType type) const
{
    stats.blockReads++;
    return storage.at(nr, type);
}

const FSBlock &
FileSystem::at(Block nr, std::vector<FSBlockType> types) const
{
    stats.blockReads++;
    return storage.at(nr, types);
}

FSBlock &
FileSystem::operator[](size_t nr)
{
    stats.blockReads++;
    return storage[nr];
}

const FSBlock &
FileSystem::operator[](size_t nr) const
{
    stats.blockReads++;
    return storage[nr];
}

FSBlockType
FileSystem::typeof(Block nr) const noexcept
{
    return storage.getType(nr);
}

FSItemType
FileSystem::typeof(Block nr, isize pos) const noexcept
{
    return storage.read(nr) ? storage[nr].itemType(pos) : FSItemType::UNUSED;
}

string
FileSystem::ascii(Block nr, isize offset, isize len) const noexcept
{
    assert(offset + len <= traits.bsize);

    return  util::createAscii(storage[nr].data() + offset, len);
}

bool
FileSystem::isUnallocated(Block nr) const noexcept
{
    assert(isize(nr) < traits.blocks);

    // The first two blocks are always allocated and not part of the bitmap
    if (nr < 2) return false;

    // Locate the allocation bit in the bitmap block
    isize byte, bit;
    auto *bm = locateAllocationBit(nr, &byte, &bit);

    // Read the bit
    return bm ? GET_BIT(bm->data()[byte], bit) : false;
}

FSBlock *
FileSystem::locateAllocationBit(Block nr, isize *byte, isize *bit) noexcept
{
    assert(isize(nr) < traits.blocks);

    // The first two blocks are always allocated and not part of the map
    if (nr < 2) return nullptr;
    nr -= 2;

    // Locate the bitmap block which stores the allocation bit
    isize bitsPerBlock = (traits.bsize - 4) * 8;
    isize bmNr = nr / bitsPerBlock;

    // Get the bitmap block
    FSBlock *bm = (bmNr < (isize)bmBlocks.size()) ? read(bmBlocks[bmNr], FSBlockType::BITMAP) : nullptr;
    if (!bm) {
        warn("Failed to lookup allocation bit for block %d (%ld)\n", nr, bmNr);
        return nullptr;
    }

    // Locate the byte position (note: the long word ordering will be reversed)
    nr = nr % bitsPerBlock;
    isize rByte = nr / 8;

    // Rectifiy the ordering
    switch (rByte % 4) {
        case 0: rByte += 3; break;
        case 1: rByte += 1; break;
        case 2: rByte -= 1; break;
        case 3: rByte -= 3; break;
    }

    // Skip the checksum which is located in the first four bytes
    rByte += 4;
    assert(rByte >= 4 && rByte < traits.bsize);

    *byte = rByte;
    *bit = nr % 8;

    return bm;
}

const FSBlock *
FileSystem::locateAllocationBit(Block nr, isize *byte, isize *bit) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FileSystem *>(this)->locateAllocationBit(nr, byte, bit));
}

isize
FileSystem::numUnallocated() const noexcept
{
    isize result = 0;
    for (auto &it : serializeBitmap()) result += util::popcount(it);

    if (FS_DEBUG) {

        isize count = 0;
        for (isize i = 0; i < numBlocks(); i++) { if (isUnallocated(Block(i))) count++; }
        debug(true, "Unallocated blocks: Fast code: %ld Slow code: %ld\n", result, count);
        assert(count == result);
    }

    return result;
}

std::vector<u32>
FileSystem::serializeBitmap() const
{
    if (!isFormatted()) return {};

    auto longwords = ((numBlocks() - 2) + 31) / 32;
    std::vector<u32> result;
    result.reserve(longwords);

    // Iterate through all bitmap blocks
    isize j = 0;
    for (auto &it : bmBlocks) {

        if (auto *bm = read(it, FSBlockType::BITMAP); bm) {

            auto *data = bm->data();
            for (isize i = 4; i < traits.bsize; i += 4) {

                if (j == longwords) break;
                result.push_back(HI_HI_LO_LO(data[i], data[i+1], data[i+2], data[i+3]));
                j++;
            }
        }
    }

    // Zero out the superfluous bits in the last word
    if (auto bits = (numBlocks() - 2) % 32; bits && !result.empty()) {
        result.back() &= (1 << bits) - 1;
    }

    return result;
}

FSBlock &
FileSystem::parent(const FSBlock &node)
{
    auto *ptr = parent(&node);
    return ptr ? *ptr : at(node.nr);
}

const FSBlock &
FileSystem::parent(const FSBlock &node) const
{
    return const_cast<const FSBlock &>(const_cast<FileSystem *>(this)->parent(node));
}

FSBlock *
FileSystem::parent(const FSBlock *node) noexcept
{
    return node->isRoot() ? read(node->nr) : read(node->nr)->getParentDirBlock();
}

const FSBlock *
FileSystem::parent(const FSBlock *node) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FileSystem *>(this)->parent(node));
}

FSBlock *
FileSystem::seekPtr(const FSBlock *root, const FSName &name) noexcept
{
    if (!root) return nullptr;

    std::unordered_set<Block> visited;

    // Check for special tokens
    if (name == "/")  return read(rootBlock);
    if (name == "")   return read(root->nr);
    if (name == ".")  return read(root->nr);
    if (name == "..") return parent(root);

    // Only proceed if a hash table is present
    if (root->hasHashTable()) {

        // Compute the table position and read the item
        u32 hash = name.hashValue(traits.dos) % root->hashTableSize();
        u32 ref = root->getHashRef(hash);

        // Traverse the linked list until the item has been found
        while (ref && visited.find(ref) == visited.end())  {

            auto *block = read(ref, { FSBlockType::USERDIR, FSBlockType::FILEHEADER });
            if (block == nullptr) break;

            if (block->isNamed(name)) return block;

            visited.insert(ref);
            ref = block->getNextHashRef();
        }
    }
    return nullptr;
}

const FSBlock *
FileSystem::seekPtr(const FSBlock *root, const FSName &name) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FileSystem *>(this)->seekPtr(root, name));
}

FSBlock *
FileSystem::seekPtr(const FSBlock *root, const fs::path &name) noexcept
{
    if (!root) return nullptr;

    FSBlock *result = read(root->nr);
    for (const auto &it : name) { if (result) { result = seekPtr(result, FSName(it)); } }
    return result;
}

const FSBlock *
FileSystem::seekPtr(const FSBlock *root, const fs::path &name) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FileSystem *>(this)->seekPtr(root, name));
}

FSBlock *
FileSystem::seekPtr(const FSBlock *root, const string &name) noexcept
{
    if (!root) return nullptr;

    auto parts = util::split(name, '/');
    if (!name.empty() && name[0] == '/') { parts.insert(parts.begin(), "/"); }

    FSBlock *result = read(root->nr);
    for (auto &it : parts) { if (result) { result = seekPtr(result, FSName(it)); } }
    return result;
}

const FSBlock *
FileSystem::seekPtr(const FSBlock *root, const string &name) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FileSystem *>(this)->seekPtr(root, name));
}

FSBlock &
FileSystem::seek(const FSBlock &root, const FSName &name)
{
    if (auto *it = seekPtr(&root, name); it) return *it;
    throw AppError(Fault::FS_NOT_FOUND, name.cpp_str());
}

const FSBlock &
FileSystem::seek(const FSBlock &root, const FSName &name) const
{
    return const_cast<const FSBlock &>(const_cast<FileSystem *>(this)->seek(root, name));
}

FSBlock &
FileSystem::seek(const FSBlock &root, const fs::path &name)
{
    if (auto *it = seekPtr(&root, name); it) return *it;
    throw AppError(Fault::FS_NOT_FOUND, name.string());
}

const FSBlock &
FileSystem::seek(const FSBlock &root, const fs::path &name) const
{
    return const_cast<const FSBlock &>(const_cast<FileSystem *>(this)->seek(root, name));
}

FSBlock &
FileSystem::seek(const FSBlock &root, const string &name)
{
    if (auto *it = seekPtr(&root, name); it) return *it;
    throw AppError(Fault::FS_NOT_FOUND, name);
}

const FSBlock &
FileSystem::seek(const FSBlock &root, const string &name) const
{
    return const_cast<const FSBlock &>(const_cast<FileSystem *>(this)->seek(root, name));
}

std::vector<const FSBlock *>
FileSystem::find(const FSOpt &opt) const
{
    return find(pwd(), opt);
}

std::vector<const FSBlock *>
FileSystem::find(const FSBlock &root, const FSOpt &opt) const
{
    require_file_or_directory(root);
    return find(&root, opt);
}

std::vector<const FSBlock *>
FileSystem::find(const FSBlock *root, const FSOpt &opt) const
{
    if (!root) return {};
    std::unordered_set<Block> visited;
    return find(root, opt, visited);
}

std::vector<Block>
FileSystem::find(Block root, const FSOpt &opt) const
{
    return FSBlock::refs(find(read(root), opt));
}

std::vector<const FSBlock *>
FileSystem::find(const FSBlock *root, const FSOpt &opt, std::unordered_set<Block> &visited) const
{
    std::vector<const FSBlock *> result;

    // Collect all items in the hash table
    auto hashedBlocks = collectHashedBlocks(*root);

    for (auto it = hashedBlocks.begin(); it != hashedBlocks.end(); it++) {

        // Add item if accepted
        if (opt.accept(*it)) result.push_back(*it);

        // Break the loop if this block has been visited before
        if (visited.contains((*it)->nr)) throw AppError(Fault::FS_HAS_CYCLES);

        // Remember the block as visited
        visited.insert((*it)->nr);
    }

    // Search subdirectories
    if (opt.recursive) {

        for (auto &it : hashedBlocks) {

            if (it->isDirectory()) {

                auto blocks = find(it, opt, visited);
                result.insert(result.end(), blocks.begin(), blocks.end());
            }
        }
    }

    // Sort the result
    if (opt.sort) {

        std::sort(result.begin(), result.end(),
                  [](auto *b1, auto *b2) { return b1->getName() < b2->getName(); });
    }

    return result;
}

std::vector<const FSBlock *>
FileSystem::find(const FSPattern &pattern) const
{
    std::vector<Block> result;

    // Determine the directory to start searching
    auto &start = pattern.isAbsolute() ? root() : pwd();

    // Seek all files matching the provided pattern
    return find(start, pattern);
}

std::vector<const FSBlock *>
FileSystem::find(const FSBlock &root, const FSPattern &pattern) const
{
    return find(&root, pattern);
}

std::vector<const FSBlock *>
FileSystem::find(const FSBlock *root, const FSPattern &pattern) const
{
    return find(root, {
        .recursive = true,
        .filter = [&](const FSBlock &item) { return pattern.match(item.cppName()); }
    });
}

std::vector<Block>
FileSystem::find(Block root, const FSPattern &pattern) const
{
    return FSBlock::refs(find(read(root), pattern));
}

std::vector<const FSBlock *>
FileSystem::match(const FSPattern &pattern) const
{
    if (pattern.isAbsolute()) {
        return match(&root(), pattern.splitted());
    } else {
        return match(&pwd(), pattern.splitted());
    }
}

std::vector<const FSBlock *>
FileSystem::match(const FSBlock *node, const FSPattern &pattern) const
{
    if (pattern.isAbsolute()) {
        return match(&root(), pattern.splitted());
    } else {
        return match(node, pattern.splitted());
    }
}

std::vector<const FSBlock *>
FileSystem::match(const FSBlock &node, const FSPattern &pattern) const
{
    return match(&node, pattern);
}

std::vector<const FSBlock *>
FileSystem::match(const FSBlock *root, std::vector<FSPattern> patterns) const
{
    std::vector<const FSBlock *> result;

    if (patterns.empty()) return {};

    // Get all directory items
    // auto items = traverse(*root, { .recursive = false} );
    auto items = FSTree(*root, { .recursive = false} );

    // Extract the first pattern
    auto pattern = patterns.front(); patterns.erase(patterns.begin());

    if (patterns.empty()) {

        // Collect all matching items
        for (auto &item : items.children) {
            if (pattern.match(item.node->cppName())) {
                result.push_back(item.node);
            }
        }

    } else {

        // Continue by searching all matching subdirectories
        for (auto &item : items.children) {
            if (item.node->isDirectory() && pattern.match(item.node->cppName())) {
                auto subdirItems = match(item.node, patterns);
                result.insert(result.end(), subdirItems.begin(), subdirItems.end());
            }
        }
    }

    return result;
}

std::vector<Block>
FileSystem::match(Block root, const FSPattern &pattern) const
{
    return FSBlock::refs(match(read(root), pattern));
}

void
FileSystem::cd(const FSName &name)
{
    if (auto ptr = seekPtr(&pwd(), name); ptr) cd (*ptr);
    throw AppError(Fault::FS_NOT_FOUND, name.cpp_str());
}

void
FileSystem::cd(const FSBlock &path)
{
    current = path.nr;
}

void
FileSystem::cd(const string &path)
{
    if (auto ptr = seekPtr(&pwd(), path); ptr) cd (*ptr);
    throw AppError(Fault::FS_NOT_FOUND, path);
}

bool
FileSystem::exists(const FSBlock &top, const fs::path &path) const
{
    return seekPtr(&top, path) != nullptr;
}

std::vector<const FSBlock *>
FileSystem::collectDataBlocks(const FSBlock &node) const
{
    std::vector<const FSBlock *> result;

    // Iterate through all list blocks and collect all data block references
    for (auto &it : collectListBlocks(node)) {

        isize num = std::min(it->getNumDataBlockRefs(), it->getMaxDataBlockRefs());
        for (isize i = 0; i < num; i++) {
            if (auto *ptr = it->getDataBlock(i); ptr) {
                result.push_back(ptr);
            }
        }
    }
    return result;
}

std::vector<Block>
FileSystem::collectDataBlocks(Block ref) const
{
    std::vector<Block> result;
    if (auto *ptr = read(ref)) {
        for (auto &it: collectDataBlocks(*ptr)) result.push_back(it->nr);
    }
    return result;
}

std::vector<const FSBlock *>
FileSystem::collectListBlocks(const FSBlock &node) const
{
    return collect(node, [&](auto *block) { return block->getNextListBlock(); });
}

std::vector<Block>
FileSystem::collectListBlocks(const Block ref) const
{
    return collect(ref, [&](auto *block) { return block->getNextListBlock(); });
}

std::vector<Block>
FileSystem::collectHashedBlocks(Block ref, isize bucket) const
{
    std::vector<Block> result;
    if (auto *ptr = read(ref)) {
        for (auto &it: collectHashedBlocks(*ptr, bucket)) result.push_back(it->nr);
    }
    return result;
}

std::vector<const FSBlock *>
FileSystem::collectHashedBlocks(const FSBlock &node, isize bucket) const
{
    auto first = node.getHashRef((u32)bucket);
    if (auto *ptr = read(first, { FSBlockType::USERDIR, FSBlockType::FILEHEADER }); ptr) {
        return collect(*ptr, [&](auto *p) { return p->getNextHashBlock(); });
    } else {
        return {};
    }
}

std::vector<Block>
FileSystem::collectHashedBlocks(Block ref) const
{
    std::vector<Block> result;
    if (auto *ptr = read(ref)) {
        for (auto &it: collectHashedBlocks(*ptr)) result.push_back(it->nr);
    }
    return result;
}

std::vector<const FSBlock *>
FileSystem::collectHashedBlocks(const FSBlock &node) const
{
    std::vector<const FSBlock *> result;

    // Walk through all hash table buckets in reverse order
    for (isize i = (isize)node.hashTableSize() - 1; i >= 0; i--) {
        for (auto &it : collectHashedBlocks(node, i)) result.push_back(it);
    }
    return result;
}

std::vector<Block>
FileSystem::collect(const Block nr, std::function<const FSBlock *(FSBlock const *)> next) const
{
    std::vector<Block> result;
    std::unordered_set<Block> visited;

    for (auto block = read(nr); block; block = next(block)) {

        // Break the loop if this block has been visited before
        if (visited.contains(block->nr)) break;

        // Add the block
        result.push_back(block->nr);

        // Remember the block as visited
        visited.insert(block->nr);
    }

    return result;
}

std::vector<const FSBlock *>
FileSystem::collect(const FSBlock &node, std::function<const FSBlock *(const FSBlock *)> next) const
{
    std::vector<const FSBlock *> result;
    std::unordered_set<Block> visited;

    for (auto block = read(node.nr); block != nullptr; block = next(block)) {

        // Break the loop if this block has been visited before
        if (visited.contains(block->nr)) break;

        // Add the block
        result.push_back(block);

        // Remember the block as visited
        visited.insert(block->nr);
    }

    return result;
}

FSBlockType
FileSystem::predictType(Block nr, const u8 *buf) const noexcept
{
    assert(buf != nullptr);
    
    // Is it a boot block?
    if (nr == 0 || nr == 1) return FSBlockType::BOOT;
    
    // Is it a bitmap block?
    if (std::find(bmBlocks.begin(), bmBlocks.end(), nr) != bmBlocks.end())
        return FSBlockType::BITMAP;
    
    // Is it a bitmap extension block?
    if (std::find(bmExtBlocks.begin(), bmExtBlocks.end(), nr) != bmExtBlocks.end())
        return FSBlockType::BITMAP_EXT;

    // For all other blocks, check the type and subtype fields
    u32 type = FSBlock::read32(buf);
    u32 subtype = FSBlock::read32(buf + traits.bsize - 4);

    if (type == 2  && subtype == 1)       return FSBlockType::ROOT;
    if (type == 2  && subtype == 2)       return FSBlockType::USERDIR;
    if (type == 2  && subtype == (u32)-3) return FSBlockType::FILEHEADER;
    if (type == 16 && subtype == (u32)-3) return FSBlockType::FILELIST;

    // Check if this block is a data block
    if (traits.ofs()) {
        if (type == 8) return FSBlockType::DATA_OFS;
    } else {
        for (isize i = 0; i < traits.bsize; i++) if (buf[i]) return FSBlockType::DATA_FFS;
    }
    
    return FSBlockType::EMPTY;
}

void
FileSystem::require_initialized() const
{
    if (!isInitialized())   throw AppError(Fault::FS_UNINITIALIZED);
}

void
FileSystem::require_formatted() const
{
    if (!isInitialized())   throw AppError(Fault::FS_UNINITIALIZED);
    if (!isFormatted())     throw AppError(Fault::FS_UNFORMATTED);
}

void
FileSystem::require_file_or_directory(const FSBlock &node) const
{
    if (!isInitialized())   throw AppError(Fault::FS_UNINITIALIZED);
    if (!isFormatted())     throw AppError(Fault::FS_UNFORMATTED);
    if (!node.isRegular())  throw AppError(Fault::FS_NOT_A_FILE_OR_DIRECTORY);
}

void
FileSystem::createUsageMap(u8 *buffer, isize len) const
{
    storage.createUsageMap(buffer, len);
/*
    // Setup priorities
    i8 pri[12];
    pri[isize(FSBlockType::UNKNOWN_BLOCK)]      = 0;
    pri[isize(FSBlockType::EMPTY_BLOCK)]        = 1;
    pri[isize(FSBlockType::BOOT_BLOCK)]         = 8;
    pri[isize(FSBlockType::ROOT_BLOCK)]         = 9;
    pri[isize(FSBlockType::BITMAP_BLOCK)]       = 7;
    pri[isize(FSBlockType::BITMAP_EXT_BLOCK)]   = 6;
    pri[isize(FSBlockType::USERDIR_BLOCK)]      = 5;
    pri[isize(FSBlockType::FILEHEADER_BLOCK)]   = 3;
    pri[isize(FSBlockType::FILELIST_BLOCK)]     = 2;
    pri[isize(FSBlockType::DATA_BLOCK_OFS)]     = 2;
    pri[isize(FSBlockType::DATA_BLOCK_FFS)]     = 2;
    
    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = 0;
 
    // Analyze all blocks
    for (isize i = 1, max = numBlocks(); i < max; i++) {

        auto val = u8(storage.getType(Block(i)));
        // auto val = u8(blocks[i]->type);
        auto pos = i * (len - 1) / (max - 1);
        if (pri[buffer[pos]] < pri[val]) buffer[pos] = val;
        if (pri[buffer[pos]] == pri[val] && pos > 0 && buffer[pos-1] != val) buffer[pos] = val;
    }
    
    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        
        if (buffer[pos] == u8(FSBlockType::UNKNOWN_BLOCK)) {
            buffer[pos] = buffer[pos - 1];
        }
    }
 */
}

void
FileSystem::createAllocationMap(u8 *buffer, isize len) const
{
    storage.createAllocationMap(buffer, len);
    /*
    // Setup priorities
    u8 pri[4] = { 0, 1, 2, 3 };

    auto &map = doctor.diagnosis.bitmapErrors;

    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = 255;
 
    // Analyze all blocks
    for (isize i = 0, max = numBlocks(); i < max; i++) {

        u8 val = isFree(Block(i)) ? 0 : map.contains(Block(i)) ? u8(map.at(Block(i)) + 1) : 1;
        auto pos = i * (len - 1) / (max - 1);
        if (buffer[pos] == 255 || pri[buffer[pos]] < pri[val]) buffer[pos] = val;
    }
    
    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        
        if (buffer[pos] == 255) {
            buffer[pos] = buffer[pos - 1];
        }
    }
    */
}

void
FileSystem::createHealthMap(u8 *buffer, isize len) const
{
    storage.createHealthMap(buffer, len);
/*
    bool strict = true; // TODO: Allow non-strict checking

    // Setup priorities
    i8 pri[3] = { 0, 1, 2 };
 
    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = 255;

    // Mark all used blocks
    // auto &map = storage.blocks;
    // for (isize i = 0, max = numBlocks(); i < max; i++) {

    // Analyze all blocks
    for (isize i = 0, max = numBlocks(); i < max; i++) {

        auto corrupted = doctor.xray(Block(i), strict); //  storage[i].corrupted;
        auto empty = isEmpty(Block(i));
        
        u8 val = empty ? 0 : corrupted ? 2 : 1;
        auto pos = i * (len - 1) / (max - 1);
        
        if (buffer[pos] == 255 || pri[buffer[pos]] < pri[val]) buffer[pos] = val;
    }
    
    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        
        if (buffer[pos] == 255) {
            buffer[pos] = buffer[pos - 1];
        }
    }
 */
}

isize
FileSystem::nextBlockOfType(FSBlockType type, Block after) const
{
    assert(isize(after) < traits.blocks);

    isize result = after;
    
    do {
        result = (result + 1) % numBlocks();
        if (storage.getType(Block(result)) == type) return result;

    } while (result != isize(after));
    
    return -1;
}

}
