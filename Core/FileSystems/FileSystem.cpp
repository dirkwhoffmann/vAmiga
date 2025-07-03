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
FileSystem::init(FloppyDrive &dfn)
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
FileSystem::init(FileSystemDescriptor layout, u8 *buf, isize len)
{
    assert(buf);

    debug(FS_DEBUG, "Importing %ld blocks from buffer...\n", layout.numBlocks);

    // Check the cosistency of the file system descriptor
    layout.checkCompatibility();

    // Only proceed if the volume is formatted
    if (layout.dos == FSVolumeType::NODOS) throw AppError(Fault::FS_UNFORMATTED);

    // Copy layout parameters
    traits.dos      = layout.dos;
    traits.reserved = layout.numReserved;
    rootBlock       = layout.rootBlock;
    bmBlocks        = layout.bmBlocks;
    bmExtBlocks     = layout.bmExtBlocks;

    // Create all blocks
    storage.init(layout.numBlocks);
    // dealloc();

    for (isize i = 0; i < layout.numBlocks; i++) {

        const u8 *data = buf + i * bsize;

        // Determine the type of the new block
        FSBlockType type = predictBlockType((Block)i, data);

        // Create new block
        // storage.write(Block(i), FSBlock::make(this, (Block)i, type));
        storage[i].init(type);

        // Import block data
        storage[i].importBlock(data, bsize);
    }

    // Set the current directory to '/'
    current = rootBlock;

    debug(FS_DEBUG, "Success\n");
}

bool
FileSystem::isInitialized() const
{
    return numBlocks() > 0;
}

bool
FileSystem::isFormatted() const
{
    // Check if the file system is initialized
    if (!isInitialized()) return false;

    // Check the DOS type
    if (traits.dos == FSVolumeType::NODOS) return false;

    // Check if the root block is present
    if (!storage.read(rootBlock, FSBlockType::ROOT_BLOCK)) return false;
    // if (rootBlockPtr(rootBlock) == nullptr) return false;

    return true;
}

FSTraits &
FileSystem::getTraits()
{
    // traits.dos = dos;
    // traits.ofs = isOFS();
    // traits.ffs = isFFS();

    traits.blocks = numBlocks();
    traits.bytes = numBytes();
    traits.bsize = blockSize();

    return traits;
}

void
FileSystem::_dump(Category category, std::ostream &os) const
{
    using namespace util;

    switch (category) {

        case Category::Info:

            if (!isFormatted()) {
                os << "Type   Size             Used    Free" << std::endl;
            } else {
                os << "Type   Size             Used    Free    Full  Name" << std::endl;
            }
            [[fallthrough]];

        case Category::State:
        {
            auto total = isFormatted() ? numBlocks() : storage.numBlocks();
            auto used = isFormatted() ? usedBlocks() : storage.usedBlocks();
            auto free = isFormatted() ? freeBlocks() : storage.freeBlocks();
            auto fill = isFormatted() ? fillLevel() : storage.fillLevel();
            auto size = std::to_string(total) + " (x " + std::to_string(bsize) + ")";

            isFormatted() ? os << "DOS" << dec(isize(traits.dos)) << " " : os << "NODOS";
            os << "  ";
            os << std::setw(15) << std::left << std::setfill(' ') << size;
            os << "  ";
            os << std::setw(6) << std::left << std::setfill(' ') << used;
            os << "  ";
            os << std::setw(6) << std::left << std::setfill(' ') << free;
            os << "  ";
            os << std::setw(3) << std::right << std::setfill(' ') << isize(fill);
            os << "%  ";
            os << getName().c_str() << std::endl;
            break;
        }
        case Category::Properties:

            os << tab("Name");
            os << getName().cpp_str() << std::endl;
            os << tab("Created");
            os << getCreationDate() << std::endl;
            os << tab("Modified");
            os << getModificationDate() << std::endl;
            os << tab("Boot block");
            os << getBootBlockName() << std::endl;
            os << tab("Capacity");
            os << util::byteCountAsString(numBytes()) << std::endl;
            os << tab("Block size");
            os << dec(bsize) << " Bytes" << std::endl;
            os << tab("Blocks");
            os << dec(numBlocks()) << std::endl;
            os << tab("Used");
            os << dec(usedBlocks());
            os << " (" <<  std::fixed << std::setprecision(2) << fillLevel() << "%)" << std::endl;
            os << tab("Root block");
            os << dec(rootBlock) << std::endl;
            os << tab("Bitmap blocks");
            for (auto& it : bmBlocks) { os << dec(it) << " "; }
            os << std::endl;
            os << util::tab("Extension blocks");
            for (auto& it : bmExtBlocks) { os << dec(it) << " "; }
            os << std::endl;
            break;

        case Category::Blocks:

            storage.dump(Category::Blocks, os);
            break;

        default:
            break;
    }
}

void
FileSystem::cacheInfo(FSInfo &result) const
{
    result.name = getName().cpp_str();
    result.creationDate = getCreationDate();
    result.modificationDate = getModificationDate();

    /*
    result.freeBlocks = freeBlocks();
    result.usedBlocks = usedBlocks();
    result.freeBytes = freeBytes();
    result.usedBytes = usedBytes();
    result.fillLevel = fillLevel();
    */
}

void
FileSystem::cacheStats(FSStats &result) const
{

}

isize
FileSystem::freeBlocks() const
{
    isize result = 0;
    isize count = numBlocks();

    for (isize i = 0; i < count; i++) {
        if (isFree((Block)i)) result++;
    }

    return result;
}

isize
FileSystem::usedBlocks() const
{
    return numBlocks() - freeBlocks();
}

FSName
FileSystem::getName() const
{
    auto *rb = storage.read(rootBlock, FSBlockType::ROOT_BLOCK);
    return rb ? rb->getName() : FSName("");
}

string
FileSystem::getCreationDate() const
{
    auto *rb = storage.read(rootBlock, FSBlockType::ROOT_BLOCK);
    return rb ? rb->getCreationDate().str() : "";
}

string
FileSystem::getModificationDate() const
{
    auto *rb = storage.read(rootBlock, FSBlockType::ROOT_BLOCK);
    // FSBlock *rb = rootBlockPtr(rootBlock);
    return rb ? rb->getModificationDate().str() : "";
}

string
FileSystem::getBootBlockName() const
{
    return BootBlockImage(storage[0].data(), storage[1].data()).name;
}

BootBlockType
FileSystem::bootBlockType() const
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
FileSystem::blockType(Block nr) const
{
    stats.blockReads++;
    return storage.getType(nr);
}

FSItemType
FileSystem::itemType(Block nr, isize pos) const
{
    return storage.read(nr) ? storage[nr].itemType(pos) : FSItemType::UNUSED;
}

FSBlock *
FileSystem::blockPtr(Block nr) const
{
    return const_cast<FSBlock *>(storage.read(nr));
}

FSBlock *
FileSystem::hashableBlockPtr(Block nr) const
{
    if (auto *p = const_cast<FSBlock *>(storage.read(nr)); p) {
        if (p->type == FSBlockType::USERDIR_BLOCK || p->type == FSBlockType::FILEHEADER_BLOCK) {
            return p;
        }
    }
    return nullptr;
}

u8
FileSystem::readByte(Block nr, isize offset) const
{
    return (storage.read(nr) && offset < bsize) ? storage[nr].data()[offset] : 0;
}

string
FileSystem::ascii(Block nr, isize offset, isize len) const
{
    assert(offset + len <= bsize);

    return  util::createAscii(storage[nr].data() + offset, len);
}

bool
FileSystem::isFree(Block nr) const
{
    assert(isBlockNumber(nr));

    // The first two blocks are always allocated and not part of the bitmap
    if (nr < 2) return false;

    // Locate the allocation bit in the bitmap block
    isize byte, bit;
    auto *bm = locateAllocationBit(nr, &byte, &bit);

    // Read the bit
    return bm ? GET_BIT(bm->data()[byte], bit) : false;
}

FSBlock *
FileSystem::locateAllocationBit(Block nr, isize *byte, isize *bit)
{
    assert(isBlockNumber(nr));

    // The first two blocks are always allocated and not part of the map
    if (nr < 2) return nullptr;
    nr -= 2;

    // Locate the bitmap block which stores the allocation bit
    isize bitsPerBlock = (bsize - 4) * 8;
    isize bmNr = nr / bitsPerBlock;

    // Get the bitmap block
    FSBlock *bm = (bmNr < (isize)bmBlocks.size()) ? read(bmBlocks[bmNr], FSBlockType::BITMAP_BLOCK) : nullptr;
    if (bm == nullptr) {
        warn("Failed to lookup allocation bit for block %d\n", nr);
        warn("bmNr = %ld\n", bmNr);
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
    assert(rByte >= 4 && rByte < bsize);

    *byte = rByte;
    *bit = nr % 8;

    return bm;
}

const FSBlock *
FileSystem::locateAllocationBit(Block nr, isize *byte, isize *bit) const
{
    return const_cast<const FSBlock *>(const_cast<FileSystem *>(this)->locateAllocationBit(nr, byte, bit));
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
FileSystem::parent(const FSBlock *node)
{
    return node->isRoot() ? blockPtr(node->nr) : blockPtr(node->nr)->getParentDirBlock();
}

const FSBlock *
FileSystem::parent(const FSBlock *node) const
{
    return const_cast<const FSBlock *>(const_cast<FileSystem *>(this)->parent(node));
}

FSBlock *
FileSystem::seekPtr(const FSBlock *root, const FSName &name)
{
    if (!root) return nullptr;

    std::unordered_set<Block> visited;

    // Check for special tokens
    if (name == "")   return blockPtr(root->nr);
    if (name == ".")  return blockPtr(root->nr);
    if (name == "..") return parent(root);
    if (name == "/")  return blockPtr(rootBlock);

    // Only proceed if a hash table is present
    if (root->hasHashTable()) {

        // Compute the table position and read the item
        u32 hash = name.hashValue(getDos()) % root->hashTableSize();
        u32 ref = root->getHashRef(hash);

        // Traverse the linked list until the item has been found
        while (ref && visited.find(ref) == visited.end())  {

            FSBlock *block = hashableBlockPtr(ref);
            if (block == nullptr) break;

            if (block->isNamed(name)) return block;

            visited.insert(ref);
            ref = block->getNextHashRef();
        }
    }
    return nullptr;
}

const FSBlock *
FileSystem::seekPtr(const FSBlock *root, const FSName &name) const
{
    return const_cast<const FSBlock *>(const_cast<FileSystem *>(this)->seekPtr(root, name));
}

FSBlock *
FileSystem::seekPtr(const FSBlock *root, const fs::path &name)
{
    if (!root) return nullptr;

    FSBlock *result = blockPtr(root->nr);
    for (const auto &it : name) { if (result) { result = seekPtr(result, FSName(it)); } }
    return result;
}

const FSBlock *
FileSystem::seekPtr(const FSBlock *root, const fs::path &name) const
{
    return const_cast<const FSBlock *>(const_cast<FileSystem *>(this)->seekPtr(root, name));
}

FSBlock *
FileSystem::seekPtr(const FSBlock *root, const string &name)
{
    if (!root) return nullptr;

    auto parts = util::split(name, '/');

    FSBlock *result = blockPtr(root->nr);
    for (auto &it : parts) { if (result) { result = seekPtr(result, FSName(it)); } }
    return result;
}

const FSBlock *
FileSystem::seekPtr(const FSBlock *root, const string &name) const
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

std::vector<FSBlock *>
FileSystem::find(const FSPattern &pattern) const
{
    std::vector<Block> result;

    // Determine the directory to start searching
    auto &start = pattern.isAbsolute() ? root() : pwd();

    // Seek all files matching the provided pattern
    return find(start, pattern);
}

std::vector<Block>
FileSystem::find(const Block root, const FSPattern &pattern) const
{
    return FSBlock::refs(find(read(root), pattern));
}

std::vector<FSBlock *>
FileSystem::match(const FSBlock *node, const FSPattern &pattern) const
{
    if (pattern.isAbsolute()) {
        return match(&root(), pattern.splitted());
    } else {
        return match(node, pattern.splitted());
    }
}

std::vector<FSBlock *>
FileSystem::match(const FSBlock &node, const FSPattern &pattern) const
{
    return match(&node, pattern);
}

std::vector<FSBlock *>
FileSystem::match(const FSBlock *root, std::vector<FSPattern> patterns) const
{
    std::vector<FSBlock *> result;

    if (patterns.empty()) return {};

    // Get all directory items
    // auto items = traverse(*root, { .recursive = false} );
    auto items = FSTree(*root, { .recursive = false} );

    // Extract the first pattern
    auto pattern = patterns.front(); patterns.erase(patterns.begin());

    if (patterns.empty()) {

        // Collect all matching items
        for (auto &item : items.children) {
            if (pattern.match(item.node->pathName())) {
                result.push_back(item.node);
            }
        }

    } else {

        // Continue by searching all matching subdirectories
        for (auto &item : items.children) {
            if (item.node->isDirectory() && pattern.match(item.node->pathName())) {
                auto subdirItems = match(item.node, patterns);
                result.insert(result.end(), subdirItems.begin(), subdirItems.end());
            }
        }
    }

    return result;
}

std::vector<FSBlock *>
FileSystem::find(const FSBlock &root, const FSOpt &opt) const
{
    return find(&root, opt);
}

std::vector<FSBlock *>
FileSystem::find(const FSBlock *root, const FSOpt &opt) const
{
    if (!root) return {};

    if (!isInitialized()) throw AppError(Fault::FS_UNINITIALIZED);
    if (!isFormatted()) throw AppError(Fault::FS_UNFORMATTED);
    if (!root->isRegular()) throw AppError(Fault::FS_INVALID_BLOCK_TYPE);

    std::unordered_set<Block> visited;
    return find(root, opt, visited);
}

std::vector<Block>
FileSystem::find(const Block root, const FSOpt &opt) const
{
    return FSBlock::refs(find(read(root), opt));
}

std::vector<FSBlock *>
FileSystem::find(const FSBlock *root, const FSOpt &opt, std::unordered_set<Block> &visited) const
{
    std::vector<FSBlock *> result;

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
                  [](FSBlock *b1, FSBlock *b2) { return b1->getName() < b2->getName(); });
    }

    return result;
}

std::vector<FSBlock *>
FileSystem::find(const FSBlock &root, const FSPattern &pattern) const
{
    return find(&root, pattern);
}

std::vector<FSBlock *>
FileSystem::find(const FSBlock *root, const FSPattern &pattern) const
{
    return find(root, {
        .recursive = true,
        .filter = [&](const FSBlock &item) { return pattern.match(item.pathName()); }
    });
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

std::vector<FSBlock *>
FileSystem::collectDataBlocks(const FSBlock &node) const
{
    std::vector<FSBlock *> result;

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
    if (auto *ptr = blockPtr(ref)) {
        for (auto &it: collectDataBlocks(*ptr)) result.push_back(it->nr);
    }
    return result;
}

std::vector<FSBlock *>
FileSystem::collectListBlocks(const FSBlock &node) const
{
    return collect(node, [&](FSBlock *block) { return block->getNextListBlock(); });
}

std::vector<Block>
FileSystem::collectListBlocks(const Block ref) const
{
    return collect(ref, [&](FSBlock *block) { return block->getNextListBlock(); });
}

std::vector<Block>
FileSystem::collectHashedBlocks(Block ref, isize bucket) const
{
    std::vector<Block> result;
    if (auto *ptr = blockPtr(ref)) {
        for (auto &it: collectHashedBlocks(*ptr, bucket)) result.push_back(it->nr);
    }
    return result;
}

std::vector<FSBlock *>
FileSystem::collectHashedBlocks(const FSBlock &node, isize bucket) const
{
    if (FSBlock *ptr = hashableBlockPtr(node.getHashRef((u32)bucket)); ptr) {
        return collect(*ptr, [&](FSBlock *p) { return p->getNextHashBlock(); });
    } else {
        return {};
    }
}

std::vector<Block>
FileSystem::collectHashedBlocks(Block ref) const
{
    std::vector<Block> result;
    if (auto *ptr = blockPtr(ref)) {
        for (auto &it: collectHashedBlocks(*ptr)) result.push_back(it->nr);
    }
    return result;
}

std::vector<FSBlock *>
FileSystem::collectHashedBlocks(const FSBlock &node) const
{
    std::vector<FSBlock *> result;

    // Walk through all hash table buckets in reverse order
    for (isize i = (isize)node.hashTableSize() - 1; i >= 0; i--) {
        for (auto &it : collectHashedBlocks(node, i)) result.push_back(it);
    }
    return result;
}

void
FileSystem::list(std::ostream &os, const FSBlock &path, const FSOpt &opt) const
{
    // List the top directory
    listDirectory(os, path, opt);

    if (opt.recursive) {

        // Find all directories to list
        auto directories = find(path, {

            .recursive = true,
            .filter = [&](const FSBlock &item) { return item.isDirectory(); }
        });

        for (auto &it : directories) {

            // Print header
            os << "Directory " << it->absName() << ":" << std::endl << std::endl;

            // Print directory
            listDirectory(os, *it, opt);
        }
    }
}

void
FileSystem::listDirectory(std::ostream &os, const FSBlock &path, const FSOpt &opt) const
{
    // Find all directory items
    auto items = find(path, { .recursive = false, .filter = opt.filter } );

    // List items
    listItems(os, items, opt);
}

void
FileSystem::listItems(std::ostream &os, std::vector<FSBlock *> items, const FSOpt &opt) const
{
    isize column = 0;

    // Sort items
    if (opt.sort) {
        std::sort(items.begin(), items.end(), [&](FSBlock *a, FSBlock *b) {
            return opt.sort(*a, *b);
        });
    }

    // Collect all displayed strings
    std::vector<string> strs;
    for (auto &it : items) strs.push_back(opt.formatter(*it));

    // Determine the longest entry
    int tab = 0; for (auto &it: strs) tab = std::max(tab, int(it.length()));

    // List all items
    for (auto &item : strs) {

        // Print in two columns if the name ends with a tab character
        if (item.back() == '\t') {

            item.pop_back();
            os << std::left << std::setw(std::max(tab, 35)) << item;
            if (column++ > 0) { os << std::endl; column = 0; }

        } else {

            if (column > 0) { os << std::endl; column = 0; }
            os << std::left << std::setw(std::max(tab, 35)) << item << std::endl;
        }
    }
}

std::vector<Block>
FileSystem::collect(const Block nr, std::function<FSBlock *(FSBlock *)> next) const
{
    std::vector<Block> result;
    std::unordered_set<Block> visited;

    for (auto block = blockPtr(nr); block; block = next(block)) {

        // Break the loop if this block has been visited before
        if (visited.contains(block->nr)) break;

        // Add the block
        result.push_back(block->nr);

        // Remember the block as visited
        visited.insert(block->nr);
    }

    return result;
}

std::vector<FSBlock *>
FileSystem::collect(const FSBlock &node, std::function<FSBlock *(FSBlock *)> next) const
{
    std::vector<FSBlock *> result;
    std::unordered_set<Block> visited;

    for (auto block = blockPtr(node.nr); block != nullptr; block = next(block)) {

        // Break the loop if this block has been visited before
        if (visited.contains(block->nr)) break;

        // Add the block
        result.push_back(block);

        // Remember the block as visited
        visited.insert(block->nr);
    }

    return result;
}

std::vector<Block>
FileSystem::hashBlockChain(Block first) const
{
    std::vector<Block> result;
    std::unordered_set<Block> visited;

    FSBlock *it = blockPtr(first);
    while (it && it->isHashable() && visited.find(it->nr) == visited.end()) {

        result.push_back(it->nr);
        visited.insert(it->nr);
        it = it->getNextHashBlock();
    }

    return result;
}

bool
FileSystem::verify()
{
    // Print some debug information about the volume
    if (FS_DEBUG) dump(Category::State);

    // Check file system integrity
    doctor.xray(true);

    if (auto erroneousBlocks = doctor.diagnosis.blockErrors; !erroneousBlocks.empty()) {

        warn("Found %ld corrupted blocks\n", erroneousBlocks.size());
        if (FS_DEBUG) dump(Category::Blocks);
        return false;
    }    
    return true;
}

Fault
FileSystem::check(Block nr, isize pos, u8 *expected, bool strict) const
{
    return storage[nr].check(pos, expected, strict);
}

Fault
FileSystem::checkBlockType(Block nr, FSBlockType type) const
{
    return checkBlockType(nr, type, type);
}

Fault
FileSystem::checkBlockType(Block nr, FSBlockType type, FSBlockType altType) const
{
    FSBlockType t = blockType(nr);
    
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

FSBlockType
FileSystem::predictBlockType(Block nr, const u8 *buf) const
{
    assert(buf != nullptr);
    
    // Is it a boot block?
    if (nr == 0 || nr == 1) return FSBlockType::BOOT_BLOCK;
    
    // Is it a bitmap block?
    if (std::find(bmBlocks.begin(), bmBlocks.end(), nr) != bmBlocks.end())
        return FSBlockType::BITMAP_BLOCK;
    
    // Is it a bitmap extension block?
    if (std::find(bmExtBlocks.begin(), bmExtBlocks.end(), nr) != bmExtBlocks.end())
        return FSBlockType::BITMAP_EXT_BLOCK;

    // For all other blocks, check the type and subtype fields
    u32 type = FSBlock::read32(buf);
    u32 subtype = FSBlock::read32(buf + bsize - 4);

    if (type == 2  && subtype == 1)       return FSBlockType::ROOT_BLOCK;
    if (type == 2  && subtype == 2)       return FSBlockType::USERDIR_BLOCK;
    if (type == 2  && subtype == (u32)-3) return FSBlockType::FILEHEADER_BLOCK;
    if (type == 16 && subtype == (u32)-3) return FSBlockType::FILELIST_BLOCK;

    // Check if this block is a data block
    if (traits.ofs()) {
        if (type == 8) return FSBlockType::DATA_BLOCK_OFS;
    } else {
        for (isize i = 0; i < bsize; i++) if (buf[i]) return FSBlockType::DATA_BLOCK_FFS;
    }
    
    return FSBlockType::EMPTY_BLOCK;
}

void
FileSystem::REQUIRE_INITIALIZED() const
{
    if (!isInitialized()) throw AppError(Fault::FS_UNINITIALIZED);
}

void
FileSystem::REQUIRE_FORMATTED() const
{
    if (!isInitialized()) throw AppError(Fault::FS_UNINITIALIZED);
    if (!isFormatted()) throw AppError(Fault::FS_UNFORMATTED);
}
void
FileSystem::REQUIRE_FILE_OR_DIRECTORY(FSBlock &node) const
{
    if (!isInitialized()) throw AppError(Fault::FS_UNINITIALIZED);
    if (!isFormatted()) throw AppError(Fault::FS_UNFORMATTED);
    if (!node.isRegular()) throw AppError(Fault::FS_NOT_A_FILE_OR_DIRECTORY);
}


void
FileSystem::createUsageMap(u8 *buffer, isize len)
{
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
}

void
FileSystem::createAllocationMap(u8 *buffer, isize len)
{
    // Setup priorities
    u8 pri[4] = { 0, 1, 2, 3 };

    auto &map = doctor.diagnosis.bitmapErrors;

    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = 255;
 
    // Analyze all blocks
    for (isize i = 0, max = numBlocks(); i < max; i++) {

        u8 val = isFree(Block(i)) ? 0 : map.contains(Block(i)) ? u8(map[Block(i)] + 1) : 1;
        auto pos = i * (len - 1) / (max - 1);
        if (buffer[pos] == 255 || pri[buffer[pos]] < pri[val]) buffer[pos] = val;
    }
    
    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        
        if (buffer[pos] == 255) {
            buffer[pos] = buffer[pos - 1];
        }
    }
}

void
FileSystem::createHealthMap(u8 *buffer, isize len)
{
    bool strict = true; // TODO: Allow non-strict checking

    // Setup priorities
    i8 pri[3] = { 0, 1, 2 };
 
    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = 255;
 
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
}

isize
FileSystem::nextBlockOfType(FSBlockType type, isize after) const
{
    assert(isBlockNumber(after));

    isize result = after;
    
    do {
        result = (result + 1) % numBlocks();
        if (storage.getType(Block(result)) == type) return result;
        // if (blocks[result]->type == type) return result;

    } while (result != after);
    
    return -1;
}

}
