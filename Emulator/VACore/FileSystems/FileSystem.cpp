// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "IOUtils.h"
#include "MutableFileSystem.h"
#include "MemUtils.h"

#include <climits>
#include <set>
#include <stack>
#include <algorithm>

namespace vamiga {

FileSystem::~FileSystem()
{
    for (auto &b : blocks) delete b;
}

void 
FileSystem::init(const MediaFile &file, isize part) throws
{
    switch (file.type()) {

        case FileType::ADF:  init(dynamic_cast<const ADFFile &>(file)); break;
        case FileType::HDF:  init(dynamic_cast<const HDFFile &>(file), part); break;

        default:
            throw CoreError(Fault::FILE_TYPE_UNSUPPORTED);
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
    if (layout.dos == FSVolumeType::NODOS) throw CoreError(Fault::FS_UNFORMATTED);

    // Copy layout parameters
    dos         = layout.dos;
    numReserved = layout.numReserved;
    rootBlock   = layout.rootBlock;
    bmBlocks    = layout.bmBlocks;
    bmExtBlocks = layout.bmExtBlocks;
    
    // Create all blocks
    assert(blocks.empty());
    for (isize i = 0; i < layout.numBlocks; i++) {
        
        const u8 *data = buf + i * bsize;

        // Determine the type of the new block
        FSBlockType type = predictBlockType((Block)i, data);
        
        // Create new block
        blocks.push_back(FSBlock::make(*this, (Block)i, type));

        // Import block data
        blocks[i]->importBlock(data, bsize);
    }
    
    // Set the current directory to '/'
    cd = rootBlock;
    
    // Print some debug information
    debug(FS_DEBUG, "Success\n");
    if (FS_DEBUG) printDirectory(true);
}

FSTraits &
FileSystem::getTraits()
{
    traits.dos = dos;
    traits.ofs = isOFS();
    traits.ffs = isFFS();

    traits.blocks = numBlocks();
    traits.bytes = numBytes();
    traits.bsize = blockSize();

    return traits;
}

void
FileSystem::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::State) {
        
        auto total = numBlocks();
        auto used = usedBlocks();
        auto free = freeBlocks();
        auto fill = (isize)(100.0 * used / total);
        
        os << "DOS" << dec(isize(dos));
        os << "   ";
        os << std::setw(6) << std::left << std::setfill(' ') << total;
        os << " (x ";
        os << std::setw(3) << std::left << std::setfill(' ') << bsize;
        os << ")  ";
        os << std::setw(6) << std::left << std::setfill(' ') << used;
        os << "  ";
        os << std::setw(6) << std::left << std::setfill(' ') << free;
        os << "  ";
        os << std::setw(3) << std::right << std::setfill(' ') << fill;
        os << "%  ";
        os << getName().c_str() << std::endl;
    }
    
    if (category == Category::Properties) {
        
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
    }
    
    if (category == Category::Blocks) {
        
        for (isize i = 0; i < numBlocks(); i++)  {
            
            if (blocks[i]->type == FSBlockType::EMPTY_BLOCK) continue;
            
            msg("\nBlock %ld (%d):", i, blocks[i]->nr);
            msg(" %s\n", FSBlockTypeEnum::key(blocks[i]->type));
            
            blocks[i]->dump();
        }
    }
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
    FSBlock *rb = rootBlockPtr(rootBlock);
    return rb ? rb->getName() : FSName("");
}

string
FileSystem::getCreationDate() const
{
    FSBlock *rb = rootBlockPtr(rootBlock);
    return rb ? rb->getCreationDate().str() : "";
}

string
FileSystem::getModificationDate() const
{
    FSBlock *rb = rootBlockPtr(rootBlock);
    return rb ? rb->getModificationDate().str() : "";
}

string
FileSystem::getBootBlockName() const
{
    return BootBlockImage(blocks[0]->data.ptr, blocks[1]->data.ptr).name;
}

BootBlockType
FileSystem::bootBlockType() const
{
    return BootBlockImage(blocks[0]->data.ptr, blocks[1]->data.ptr).type;
}

FSBlockType
FileSystem::blockType(Block nr) const
{
    return blockPtr(nr) ? blocks[nr]->type : FSBlockType::UNKNOWN_BLOCK;
}

FSItemType
FileSystem::itemType(Block nr, isize pos) const
{
    return blockPtr(nr) ? blocks[nr]->itemType(pos) : FSItemType::UNUSED;
}

FSBlock *
FileSystem::blockPtr(Block nr) const
{
    return nr < blocks.size() ? blocks[nr] : nullptr;
}

FSBlock *
FileSystem::bootBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FSBlockType::BOOT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::rootBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FSBlockType::ROOT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::bitmapBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FSBlockType::BITMAP_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::bitmapExtBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FSBlockType::BITMAP_EXT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::userDirBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FSBlockType::USERDIR_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::fileHeaderBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FSBlockType::FILEHEADER_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::fileListBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FSBlockType::FILELIST_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::dataBlockPtr(Block nr) const
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type : FSBlockType::UNKNOWN_BLOCK;

    if (t == FSBlockType::DATA_BLOCK_OFS || t == FSBlockType::DATA_BLOCK_FFS) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::hashableBlockPtr(Block nr) const
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type : FSBlockType::UNKNOWN_BLOCK;
    
    if (t == FSBlockType::USERDIR_BLOCK || t == FSBlockType::FILEHEADER_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

u8
FileSystem::readByte(Block nr, isize offset) const
{
    assert(offset < bsize);

    if (isize(nr) < numBlocks()) {
        return blocks[nr]->data.ptr ? blocks[nr]->data[offset] : 0;
    }
    
    return 0;
}

string
FileSystem::ascii(Block nr, isize offset, isize len) const
{
    assert(isBlockNumber(nr));
    assert(offset + len <= bsize);
    
    if (blocks[nr]->data.ptr) {
        return util::createAscii(blocks[nr]->data.ptr + offset, len);
    } else {
        return string(len, '.');
    }
}

bool
FileSystem::isFree(Block nr) const
{
    assert(isBlockNumber(nr));

    // The first two blocks are always allocated and not part of the bitmap
    if (nr < 2) return false;
    
    // Locate the allocation bit in the bitmap block
    isize byte, bit;
    FSBlock *bm = locateAllocationBit(nr, &byte, &bit);

    // Read the bit
    return bm ? GET_BIT(bm->data[byte], bit) : false;
}

FSBlock *
FileSystem::locateAllocationBit(Block nr, isize *byte, isize *bit) const
{
    assert(isBlockNumber(nr));

    // The first two blocks are always allocated and not part of the map
    if (nr < 2) return nullptr;
    nr -= 2;
    
    // Locate the bitmap block which stores the allocation bit
    isize bitsPerBlock = (bsize - 4) * 8;
    isize bmNr = nr / bitsPerBlock;

    // Get the bitmap block
    FSBlock *bm;
    bm = (bmNr < (isize)bmBlocks.size()) ? bitmapBlockPtr(bmBlocks[bmNr]) : nullptr;
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
    
    // debug(FS_DEBUG, "Alloc bit for %d: block: %d byte: %d bit: %d\n",
    //       ref, bm->nr, *byte, *bit);

    return bm;
}

FSBlock *
FileSystem::currentDirBlock() const
{
    FSBlock *cdb = blockPtr(cd);
    
    assert(cdb != nullptr);
    assert(cdb->type == FSBlockType::ROOT_BLOCK || cdb->type == FSBlockType::USERDIR_BLOCK);
    
    return cdb;

    /*
    if (cdb) {
        if (cdb->type == FSBlockType::ROOT_BLOCK || cdb->type == FSBlockType::USERDIR_BLOCK) {
            return cdb;
        }
    }
    
    // The block reference is invalid. Switch back to the root directory
    cd = rootBlock;
    return blockPtr(cd);
    */
}

FSBlock *
FileSystem::changeDir(const string &name)
{
    FSBlock *cdb = currentDirBlock();
    
    if (name == "/") {
        
        // Move to top level
        cd = rootBlock;
        
    } else if (name == "..") {
        
        // Move one level up
        cd = cdb->getParentDirRef();
        
    } else if (FSBlock *subdir = seekDir(name); subdir) {
        
        // Move one level down
        cd = subdir->nr;
    }
    
    if (FSBlock *result = blockPtr(cd); result) {
        
        // Make sure we are still at a directory block
        if (result->type == FSBlockType::ROOT_BLOCK)    return result;
        if (result->type == FSBlockType::USERDIR_BLOCK) return result;
    }
    
    // Switch back to the root directory, as the reference is invalid
    cd = rootBlock;
    return blockPtr(cd);
}

void
FileSystem::printDirectory(bool recursive) const
{
    std::vector<Block> items;
    collect(cd, items);
    
    for (auto const& i : items) {
        msg("%s\n", getPath(i).c_str());
    }
    msg("%zu items\n", items.size());
}

string
FileSystem::getPath(FSBlock *block) const
{
    string result = "";
    std::set<Block> visited;

    while(block) {

        // Break the loop if this block has an invalid type
        if (!hashableBlockPtr(block->nr)) break;

        // Break the loop if this block was visited before
        if (visited.find(block->nr) != visited.end()) break;
        
        // Add the block to the set of visited blocks
        visited.insert(block->nr);

        // Expand the path
        string name = block->getName().c_str();
        result = (result == "") ? name : name + "/" + result;
        
        // Continue with the parent block
        block = block->getParentDirBlock();
    }
    
    return result;
}

Block
FileSystem::seekRef(FSName name) const
{
    std::set<Block> visited;
    
    // Only proceed if a hash table is present
    FSBlock *cdb = currentDirBlock();
    if (!cdb || cdb->hashTableSize() == 0) return 0;
    
    // Compute the table position and read the item
    u32 hash = name.hashValue() % cdb->hashTableSize();
    u32 ref = cdb->getHashRef(hash);
    
    // Traverse the linked list until the item has been found
    while (ref && visited.find(ref) == visited.end())  {
        
        FSBlock *item = hashableBlockPtr(ref);
        if (item == nullptr) break;
        
        if (item->isNamed(name)) return item->nr;

        visited.insert(ref);
        ref = item->getNextHashRef();
    }

    return 0;
}

FSBlock *
FileSystem::seekPath(const fs::path &path)
{
    FSBlock *block = nullptr;

    changeDir("/");

    for (const auto& part : path) {
        
        if (part == path.filename()) {
            block = seekFile(part.string());
        } else {
            block = changeDir(part.string());
        }
        
        if (!block) break;
    }
    
    return block;
}

void
FileSystem::collect(Block nr, std::vector<Block> &result, bool recursive) const
{
    std::stack<Block> remainingItems;
    std::set<Block> visited;
    
    // Start with the items in this block
    collectHashedRefs(nr, remainingItems, visited);
    
    // Move the collected items to the result list
    while (remainingItems.size() > 0) {
        
        Block item = remainingItems.top();
        remainingItems.pop();
        result.push_back(item);

        // Add subdirectory items to the queue
        if (userDirBlockPtr(item) && recursive) {
            collectHashedRefs(item, remainingItems, visited);
        }
    }
}

void
FileSystem::collectHashedRefs(Block nr,
                              std::stack<Block> &result, std::set<Block> &visited) const
{
    if (FSBlock *b = blockPtr(nr)) {
        
        // Walk through the hash table in reverse order
        for (isize i = (isize)b->hashTableSize(); i >= 0; i--) {
            collectRefsWithSameHashValue(b->getHashRef((u32)i), result, visited);
        }
    }
}

void
FileSystem::collectRefsWithSameHashValue(Block nr,
                                         std::stack<Block> &result, std::set<Block> &visited) const
{
    std::stack<Block> refs;
    
    // Walk down the linked list
    for (FSBlock *b = hashableBlockPtr(nr); b; b = b->getNextHashBlock()) {

        // Only proceed if we haven't seen this block yet
        if (visited.find(b->nr) != visited.end()) throw CoreError(Fault::FS_HAS_CYCLES);

        visited.insert(b->nr);
        refs.push(b->nr);
    }

    // Push the collected elements onto the result stack
    while (refs.size() > 0) { result.push(refs.top()); refs.pop(); }
}

FSBlock *
FileSystem::lastFileListBlockInChain(Block start) const
{
    FSBlock *block = fileListBlockPtr(start);
    return block ? lastFileListBlockInChain(block) : nullptr;
}

FSBlock *
FileSystem::lastFileListBlockInChain(FSBlock *block) const
{
    std::set<Block> visited;

    while (block && visited.find(block->nr) == visited.end()) {

        FSBlock *next = block->getNextListBlock();
        if (next == nullptr) return block;

        visited.insert(block->nr);
        block = next;
    }
    return nullptr;
}

FSBlock *
FileSystem::lastHashBlockInChain(Block start) const
{
    FSBlock *block = hashableBlockPtr(start);
    return block ? lastHashBlockInChain(block) : nullptr;
}

FSBlock *
FileSystem::lastHashBlockInChain(FSBlock *block) const
{
    std::set<Block> visited;

    while (block && visited.find(block->nr) == visited.end()) {

        FSBlock *next = block->getNextHashBlock();
        if (next == nullptr) return block;

        visited.insert(block->nr);
        block =next;
    }
    return nullptr;
}

bool
FileSystem::verify() const
{
    if (FS_DEBUG) {
        
        dump(Category::State);
        printDirectory(true);
    }
    if (FSErrorReport report = check(true); report.corruptedBlocks > 0) {
        
        warn("Found %ld corrupted blocks\n", report.corruptedBlocks);
        if (FS_DEBUG) dump(Category::Blocks);
        return false;
    }
    
    return true;
}

FSErrorReport
FileSystem::check(bool strict) const
{
    FSErrorReport result = { };

    isize total = 0, min = INT_MAX, max = 0;
    
    // Analyze the allocation table
    for (isize i = 0; i < numBlocks(); i++) {

        FSBlock *block = blocks[i];
        if (block->type == FSBlockType::EMPTY_BLOCK && !isFree(Block(i))) {
            result.bitmapErrors++;
            debug(FS_DEBUG, "Empty block %ld is marked as allocated\n", i);
        }
        if (block->type != FSBlockType::EMPTY_BLOCK && isFree(Block(i))) {
            result.bitmapErrors++;
            debug(FS_DEBUG, "Non-empty block %ld is marked as free\n", i);
        }
    }

    // Analyze all blocks
    for (isize i = 0; i < numBlocks(); i++) {

        if (blocks[i]->check(strict) > 0) {
            min = std::min(min, i);
            max = std::max(max, i);
            blocks[i]->corrupted = ++total;
        } else {
            blocks[i]->corrupted = 0;
        }
    }

    // Record findings
    if (total) {
        result.corruptedBlocks = total;
        result.firstErrorBlock = min;
        result.lastErrorBlock = max;
    } else {
        result.corruptedBlocks = 0;
        result.firstErrorBlock = min;
        result.lastErrorBlock = max;
    }
    
    return result;
}

Fault
FileSystem::check(Block nr, isize pos, u8 *expected, bool strict) const
{
    return blocks[nr]->check(pos, expected, strict);
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

isize
FileSystem::getCorrupted(Block nr) const
{
    return blockPtr(nr) ? blocks[nr]->corrupted : 0;
}

bool
FileSystem::isCorrupted(Block nr, isize n) const
{
    for (isize i = 0, cnt = 0; i < numBlocks(); i++) {
        
        if (isCorrupted((Block)i)) {
            cnt++;
            if ((i64)nr == i) return cnt == n;
        }
    }
    return false;
}

Block
FileSystem::nextCorrupted(Block nr) const
{
    isize i = (isize)nr;
    while (++i < numBlocks()) { if (isCorrupted((Block)i)) return (Block)i; }
    return nr;
}

Block
FileSystem::prevCorrupted(Block nr) const
{
    isize i = (isize)nr - 1;
    while (i-- >= 0) { if (isCorrupted((Block)i)) return (Block)i; }
    return nr;
}

Block
FileSystem::seekCorruptedBlock(isize n) const
{
    for (isize i = 0, cnt = 0; i < numBlocks(); i++) {

        if (isCorrupted((Block)i)) {
            cnt++;
            if (cnt == n) return (Block)i;
        }
    }
    return (Block)-1;
}

FSBlockType
FileSystem::predictBlockType(Block nr, const u8 *buffer) const
{
    assert(buffer != nullptr);
    
    // Is it a boot block?
    if (nr == 0 || nr == 1) return FSBlockType::BOOT_BLOCK;
    
    // Is it a bitmap block?
    if (std::find(bmBlocks.begin(), bmBlocks.end(), nr) != bmBlocks.end())
        return FSBlockType::BITMAP_BLOCK;
    
    // Is it a bitmap extension block?
    if (std::find(bmExtBlocks.begin(), bmExtBlocks.end(), nr) != bmExtBlocks.end())
        return FSBlockType::BITMAP_EXT_BLOCK;

    // For all other blocks, check the type and subtype fields
    u32 type = FSBlock::read32(buffer);
    u32 subtype = FSBlock::read32(buffer + bsize - 4);

    if (type == 2  && subtype == 1)       return FSBlockType::ROOT_BLOCK;
    if (type == 2  && subtype == 2)       return FSBlockType::USERDIR_BLOCK;
    if (type == 2  && subtype == (u32)-3) return FSBlockType::FILEHEADER_BLOCK;
    if (type == 16 && subtype == (u32)-3) return FSBlockType::FILELIST_BLOCK;

    // Check if this block is a data block
    if (isOFS()) {
        if (type == 8) return FSBlockType::DATA_BLOCK_OFS;
    } else {
        for (isize i = 0; i < bsize; i++) if (buffer[i]) return FSBlockType::DATA_BLOCK_FFS;
    }
    
    return FSBlockType::EMPTY_BLOCK;
}

void
FileSystem::analyzeBlockUsage(u8 *buffer, isize len)
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
    pri[isize(FSBlockType::FILEHEADER_BLOCK)]   = 4;
    pri[isize(FSBlockType::FILELIST_BLOCK)]     = 3;
    pri[isize(FSBlockType::DATA_BLOCK_OFS)]     = 2;
    pri[isize(FSBlockType::DATA_BLOCK_FFS)]     = 2;
    
    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = 0;
 
    // Analyze all blocks
    for (isize i = 0, max = numBlocks(); i < max; i++) {

        auto val = u8(blocks[i]->type);
        auto pos = i * (len - 1) / (max - 1);
        if (pri[buffer[pos]] < pri[val]) buffer[pos] = val;
    }
    
    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        
        if (buffer[pos] == u8(FSBlockType::UNKNOWN_BLOCK)) {
            buffer[pos] = buffer[pos - 1];
        }
    }
}

void
FileSystem::analyzeBlockAllocation(u8 *buffer, isize len)
{
    // Setup priorities
    i8 pri[5] = { 1, 2, 3, 4, 0 };
 
    // Start with the value representing "uninitialized"
    for (isize i = 0; i < len; i++) buffer[i] = 4;
 
    // Analyze all blocks
    for (isize i = 0, max = numBlocks(); i < max; i++) {
        
        auto free = isFree(Block(i));
        auto empty = blocks[i]->type == FSBlockType::EMPTY_BLOCK;
        u8 val = (!empty && !free) ? 1 : (empty && !free) ? 2 : (!empty && free) ? 3 : 0;
        auto pos = i * (len - 1) / (max - 1);
        if (pri[buffer[pos]] < pri[val]) buffer[pos] = val;
    }
    
    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        
        if (buffer[pos] == 4) {
            buffer[pos] = buffer[pos - 1];
        }
    }
}

void
FileSystem::analyzeBlockConsistency(u8 *buffer, isize len)
{
    // Setup priorities
    i8 pri[4] = { 1, 2, 3, 0 };
 
    // Start with the value representing "uninitialized"
    for (isize i = 0; i < len; i++) buffer[i] = 4;
 
    // Analyze all blocks
    for (isize i = 0, max = numBlocks(); i < max; i++) {
        
        u8 val =
        blocks[i]->corrupted ? 2 :
        blocks[i]->type == FSBlockType::UNKNOWN_BLOCK ? 0 :
        blocks[i]->type == FSBlockType::EMPTY_BLOCK ? 0 : 1;
        auto pos = i * (len - 1) / (max - 1);
        if (pri[buffer[pos]] < pri[val]) buffer[pos] = val;
    }
    
    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        
        if (buffer[pos] == 4) {
            buffer[pos] = buffer[pos - 1];
        }
    }
}

isize
FileSystem::nextBlockOfType(FSBlockType type, isize after)
{
    assert(isBlockNumber(after));

    isize result = after;
    
    do {
        result = (result + 1) % numBlocks();
        if (blocks[result]->type == type) return result;
        
    } while (result != after);
    
    return -1;
}

isize
FileSystem::nextCorruptedBlock(isize after)
{
    assert(isBlockNumber(after));
    
    isize result = after;
    
    do {
        result = (result + 1) % numBlocks();
        if (blocks[result]->corrupted) return result;
        
    } while (result != after);
    
    return -1;
}

}
