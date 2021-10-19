// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IO.h"
#include "FSDevice.h"
#include "MemUtils.h"
#include <climits>
#include <set>
#include <stack>

void
FSDevice::init(isize capacity)
{
    assert(blocks.empty());
    
    blocks.reserve(capacity);
    blocks.assign(capacity, 0);
}

void
FSDevice::init(FSDeviceDescriptor &layout)
{
    init(layout.numBlocks);
    
    if constexpr (FS_DEBUG) { layout.dump(); }
    
    // Copy layout parameters from descriptor
    numCyls    = layout.numCyls;
    numHeads   = layout.numHeads;
    numSectors = layout.numSectors;
    bsize      = layout.bsize;
    numBlocks  = layout.numBlocks;
        
    // Create all partitions
    for (auto& descriptor : layout.partitions) {
        partitions.push_back(new FSPartition(*this, descriptor));
    }

    // Compute checksums for all blocks
    updateChecksums();
    
    // Set the current directory to '/'
    cd = partitions[0]->rootBlock;
    
    // Do some consistency checking
    for (isize i = 0; i < numBlocks; i++) assert(blocks[i] != nullptr);
    
    // Print some debug information
    if constexpr (FS_DEBUG) { info(); dump(); }
}

void
FSDevice::init(DiskDiameter dia, DiskDensity den)
{
    // Get a device descriptor
    auto descriptor = FSDeviceDescriptor(dia, den);
        
    // Create the device
    init(descriptor);
}

void
FSDevice::init(DiskDiameter dia, DiskDensity den, const string &path)
{
    init(dia, den);
    
    // Try to import directory
    importDirectory(path);
    
    // Assign device name
    setName(FSName("Directory")); // TODO: Use last path component

    // Compute checksums for all blocks
    updateChecksums();

    // Change to the root directory
    changeDir("/");
}

void
FSDevice::init(ADFFile &adf)
{
    // Get a device descriptor for the ADF
    FSDeviceDescriptor descriptor = adf.layout();
        
    // Create the device
    init(descriptor);

    // Import file system from ADF
    importVolume(adf.data, adf.size);
}

void
FSDevice::init(HDFFile &hdf)
{
    // Get a device descriptor for the HDF
    FSDeviceDescriptor descriptor = hdf.layout();

    // Create the device
    init(descriptor);

    // Import file system from HDF
    info();
    // importVolume(adf.data, adf.size);
}

void
FSDevice::init(FSVolumeType type, const string &path)
{
    // Try to fit the directory into files system with DD disk capacity
    try { init(INCH_35, DISK_DD, path); return; } catch (...) { };

    // Try to fit the directory into files system with HD disk capacity
    init(INCH_35, DISK_HD, path);
}

FSDevice::~FSDevice()
{
    for (auto &p : partitions) delete p;
    for (auto &b : blocks) delete b;
}

void
FSDevice::info()
{
    msg("Type    Size           Used    Free   Full   Name\n");
    for (auto& p : partitions) p->info();
}

void
FSDevice::_dump(dump::Category category, std::ostream& os) const
{
    // Dump all partitions
    for (auto &p : partitions) {
        p->dump();
    }
    msg("\n");

    // Dump all blocks
    for (isize i = 0; i < numBlocks; i++)  {
        
        if (blocks[i]->type == FS_EMPTY_BLOCK) continue;
        
        msg("\nBlock %zu (%d):", i, blocks[i]->nr);
        msg(" %s\n", FSBlockTypeEnum::key(blocks[i]->type));
                
        blocks[i]->dump(); 
    }
}

isize
FSDevice::partitionForBlock(Block nr)
{
    for (isize i = 0; i < (isize)partitions.size(); i++) {
        if (nr >= partitions[i]->firstBlock && nr <= partitions[i]->lastBlock) {
            return i;
        }
    }
    
    fatalError;
}

FSBlockType
FSDevice::blockType(Block nr)
{
    return blockPtr(nr) ? blocks[nr]->type : FS_UNKNOWN_BLOCK;
}

FSItemType
FSDevice::itemType(Block nr, isize pos) const
{
    return blockPtr(nr) ? blocks[nr]->itemType(pos) : FSI_UNUSED;
}

FSBlock *
FSDevice::blockPtr(Block nr) const
{
    return nr < blocks.size() ? blocks[nr] : nullptr;
}

FSBlock *
FSDevice::bootBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BOOT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::rootBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_ROOT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::bitmapBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BITMAP_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::bitmapExtBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BITMAP_EXT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::userDirBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_USERDIR_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::fileHeaderBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_FILEHEADER_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::fileListBlockPtr(Block nr)
{
    if (nr < blocks.size() && blocks[nr]->type == FS_FILELIST_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::dataBlockPtr(Block nr)
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type : FS_UNKNOWN_BLOCK;

    if (t == FS_DATA_BLOCK_OFS || t == FS_DATA_BLOCK_FFS) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::hashableBlockPtr(Block nr)
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type : FS_UNKNOWN_BLOCK;
    
    if (t == FS_USERDIR_BLOCK || t == FS_FILEHEADER_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

void
FSDevice::updateChecksums()
{
    for (isize i = 0; i < numBlocks; i++) {
        blocks[i]->updateChecksum();
    }
}

FSBlock *
FSDevice::currentDirBlock()
{
    FSBlock *cdb = blockPtr(cd);
    
    if (cdb) {
        if (cdb->type == FS_ROOT_BLOCK || cdb->type == FS_USERDIR_BLOCK) {
            return cdb;
        }
    }
    
    // The block reference is invalid. Switch back to the root directory
    cd = partitions[cp]->rootBlock;
    return blockPtr(cd);
}

FSBlock *
FSDevice::changeDir(const string &name)
{
    FSBlock *cdb = currentDirBlock();

    if (name == "/") {
                
        // Move to top level
        cd = partitions[cp]->rootBlock;
        return currentDirBlock();
    }

    if (name == "..") {
                
        // Move one level up
        cd = cdb->getParentDirRef();
        return currentDirBlock();
    }
    
    FSBlock *subdir = seekDir(name);
    if (subdir == nullptr) return cdb;
    
    // Move one level down
    cd = subdir->nr;
    return currentDirBlock();
}

string
FSDevice::getPath(FSBlock *block)
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

FSBlock *
FSDevice::createDir(const string &name)
{
    FSBlock *cdb = currentDirBlock();
    FSBlock *block = cdb->partition.newUserDirBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParentDirRef(cdb->nr);
    addHashRef(block->nr);
    
    return block;
}

FSBlock *
FSDevice::createFile(const string &name)
{
    FSBlock *cdb = currentDirBlock();
    FSBlock *block = cdb->partition.newFileHeaderBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParentDirRef(cdb->nr);
    addHashRef(block->nr);

    return block;
}

FSBlock *
FSDevice::createFile(const string &name, const u8 *buf, isize size)
{
    assert(buf);

    FSBlock *block = createFile(name);
    
    if (block) {
        assert(block->type == FS_FILEHEADER_BLOCK);
        block->addData(buf, size);
    }
    
    return block;
}

FSBlock *
FSDevice::createFile(const string &name, const string &str)
{
    return createFile(name, (const u8 *)str.c_str(), str.size());
}

Block
FSDevice::seekRef(FSName name)
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

void
FSDevice::addHashRef(Block nr)
{
    if (FSBlock *block = hashableBlockPtr(nr)) {
        addHashRef(block);
    }
}

void
FSDevice::addHashRef(FSBlock *newBlock)
{
    // Only proceed if a hash table is present
    FSBlock *cdb = currentDirBlock();
    if (!cdb || cdb->hashTableSize() == 0) { return; }

    // Read the item at the proper hash table location
    u32 hash = newBlock->hashValue() % cdb->hashTableSize();
    u32 ref = cdb->getHashRef(hash);

    // If the slot is empty, put the reference there
    if (ref == 0) { cdb->setHashRef(hash, newBlock->nr); return; }

    // Otherwise, put it into the last element of the block list chain
    FSBlock *last = lastHashBlockInChain(ref);
    if (last) last->setNextHashRef(newBlock->nr);
}

void
FSDevice::printDirectory(bool recursive)
{
    std::vector<Block> items;
    collect(cd, items);
    
    for (auto const& i : items) {
        msg("%s\n", getPath(i).c_str());
    }
    msg("%zu items\n", items.size());
}


FSBlock *
FSDevice::lastHashBlockInChain(Block start)
{
    FSBlock *block = hashableBlockPtr(start);
    return block ? lastHashBlockInChain(block) : nullptr;
}

FSBlock *
FSDevice::lastHashBlockInChain(FSBlock *block)
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

FSBlock *
FSDevice::lastFileListBlockInChain(Block start)
{
    FSBlock *block = fileListBlockPtr(start);
    return block ? lastFileListBlockInChain(block) : nullptr;
}

FSBlock *
FSDevice::lastFileListBlockInChain(FSBlock *block)
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

void
FSDevice::collect(Block nr, std::vector<Block> &result, bool recursive)
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
FSDevice::collectHashedRefs(Block nr,
                            std::stack<Block> &result, std::set<Block> &visited)
{
    if (FSBlock *b = blockPtr(nr)) {
        
        // Walk through the hash table in reverse order
        for (isize i = (isize)b->hashTableSize(); i >= 0; i--) {
            collectRefsWithSameHashValue(b->getHashRef((u32)i), result, visited);
        }
    }
}

void
FSDevice::collectRefsWithSameHashValue(Block nr,
                                       std::stack<Block> &result, std::set<Block> &visited)
{
    std::stack<Block> refs;
    
    // Walk down the linked list
    for (FSBlock *b = hashableBlockPtr(nr); b; b = b->getNextHashBlock()) {

        // Only proceed if we haven't seen this block yet
        if (visited.find(b->nr) != visited.end()) throw VAError(ERROR_FS_HAS_CYCLES);

        visited.insert(b->nr);
        refs.push(b->nr);
    }
  
    // Push the collected elements onto the result stack
    while (refs.size() > 0) { result.push(refs.top()); refs.pop(); }
}

FSErrorReport
FSDevice::check(bool strict) const
{
    FSErrorReport result;

    isize total = 0, min = SSIZE_MAX, max = 0;
    
    // Analyze all partions
    for (auto &p : partitions) p->check(strict, result);

    // Analyze all blocks
    for (isize i = 0; i < numBlocks; i++) {

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

ErrorCode
FSDevice::check(Block nr, isize pos, u8 *expected, bool strict) const
{
    return blocks[nr]->check(pos, expected, strict);
}

ErrorCode
FSDevice::checkBlockType(Block nr, FSBlockType type)
{
    return checkBlockType(nr, type, type);
}

ErrorCode
FSDevice::checkBlockType(Block nr, FSBlockType type, FSBlockType altType)
{
    FSBlockType t = blockType(nr);
    
    if (t != type && t != altType) {
        
        switch (t) {
                
            case FS_EMPTY_BLOCK:      return ERROR_FS_PTR_TO_EMPTY_BLOCK;
            case FS_BOOT_BLOCK:       return ERROR_FS_PTR_TO_BOOT_BLOCK;
            case FS_ROOT_BLOCK:       return ERROR_FS_PTR_TO_ROOT_BLOCK;
            case FS_BITMAP_BLOCK:     return ERROR_FS_PTR_TO_BITMAP_BLOCK;
            case FS_BITMAP_EXT_BLOCK: return ERROR_FS_PTR_TO_BITMAP_EXT_BLOCK;
            case FS_USERDIR_BLOCK:    return ERROR_FS_PTR_TO_USERDIR_BLOCK;
            case FS_FILEHEADER_BLOCK: return ERROR_FS_PTR_TO_FILEHEADER_BLOCK;
            case FS_FILELIST_BLOCK:   return ERROR_FS_PTR_TO_FILELIST_BLOCK;
            case FS_DATA_BLOCK_OFS:   return ERROR_FS_PTR_TO_DATA_BLOCK;
            case FS_DATA_BLOCK_FFS:   return ERROR_FS_PTR_TO_DATA_BLOCK;
            default:                  return ERROR_FS_PTR_TO_UNKNOWN_BLOCK;
        }
    }

    return ERROR_OK;
}

isize
FSDevice::getCorrupted(Block nr)
{
    return blockPtr(nr) ? blocks[nr]->corrupted : 0;
}

bool
FSDevice::isCorrupted(Block nr, isize n)
{
    for (isize i = 0, cnt = 0; i < numBlocks; i++) {
        
        if (isCorrupted((Block)i)) {
            cnt++;
            if ((i64)nr == i) return cnt == n;
        }
    }
    return false;
}

Block
FSDevice::nextCorrupted(Block nr)
{
    isize i = (isize)nr;
    while (++i < numBlocks) { if (isCorrupted((Block)i)) return (Block)i; }
    return nr;
}

Block
FSDevice::prevCorrupted(Block nr)
{
    isize i = (isize)nr - 1;
    while (i-- >= 0) { if (isCorrupted((Block)i)) return (Block)i; }
    return nr;
}

Block
FSDevice::seekCorruptedBlock(isize n)
{
    for (isize i = 0, cnt = 0; i < numBlocks; i++) {

        if (isCorrupted((Block)i)) {
            cnt++;
            if (cnt == n) return (Block)i;
        }
    }
    return (Block)-1;
}

u8
FSDevice::readByte(Block nr, isize offset) const
{
    assert(offset < bsize);

    if (nr < (Block)numBlocks) {
        return blocks[nr]->data ? blocks[nr]->data[offset] : 0;
    }
    
    return 0;
}

FSBlockType
FSDevice::predictBlockType(Block nr, const u8 *buffer)
{
    assert(buffer != nullptr);
    
    for (auto &p : partitions) {
        if (FSBlockType t = p->predictBlockType(nr, buffer); t != FS_UNKNOWN_BLOCK) {
            return t;
        }
    }
    return FS_UNKNOWN_BLOCK;
}

void
FSDevice::importVolume(const u8 *src, isize size)
{
    assert(src != nullptr);

    debug(FS_DEBUG, "Importing file system...\n");

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) throw VAError(ERROR_FS_WRONG_BSIZE);

    // Only proceed if the source buffer contains the right amount of data
    if (numBlocks * bsize != size) throw VAError(ERROR_FS_WRONG_CAPACITY);

    // Only proceed if all partitions contain a valid file system
    for (auto &it : partitions) {
        if (it->dos == FS_NODOS) throw VAError(ERROR_FS_UNSUPPORTED);
    }
        
    // Import all blocks
    for (isize i = 0; i < numBlocks; i++) {
        
        const u8 *data = src + i * bsize;
        
        // Get the partition this block belongs to
        FSPartition &p = blocks[i]->partition;
        
        // Determine the type of the new block
        FSBlockType type = p.predictBlockType((Block)i, data);
        
        // Create new block
        FSBlock *newBlock = FSBlock::make(p, (Block)i, type);

        // Import block data
        newBlock->importBlock(data, bsize);

        // Replace the existing block
        assert(blocks[i] != nullptr);
        delete blocks[i];
        blocks[i] = newBlock;
    }
    
    // Print some debug information
    debug(FS_DEBUG, "Success\n");
    // info();
    // dump();
    // util::hexdump(blocks[0]->data, 512);
    printDirectory(true);
}

bool
FSDevice::exportVolume(u8 *dst, isize size)
{
    return exportBlocks(0, (Block)(numBlocks - 1), dst, size);
}

bool
FSDevice::exportVolume(u8 *dst, isize size, ErrorCode *err)
{
    return exportBlocks(0, (Block)(numBlocks - 1), dst, size, err);
}

bool
FSDevice::exportBlock(Block nr, u8 *dst, isize size)
{
    return exportBlocks(nr, nr, dst, size);
}

bool
FSDevice::exportBlock(Block nr, u8 *dst, isize size, ErrorCode *error)
{
    return exportBlocks(nr, nr, dst, size, error);
}

bool
FSDevice::exportBlocks(Block first, Block last, u8 *dst, isize size)
{
    ErrorCode error;
    bool result = exportBlocks(first, last, dst, size, &error);
    
    assert(result == (error == ERROR_OK));
    return result;
}

bool
FSDevice::exportBlocks(Block first, Block last, u8 *dst, isize size, ErrorCode *err)
{
    assert(last < (Block)numBlocks);
    assert(first <= last);
    assert(dst);
    
    isize count = last - first + 1;
    
    debug(FS_DEBUG, "Exporting %zd blocks (%d - %d)\n", count, first, last);

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) {
        if (err) *err = ERROR_FS_WRONG_BSIZE;
        return false;
    }

    // Only proceed if the source buffer contains the right amount of data
    if (count * bsize != size) {
        if (err) *err = ERROR_FS_WRONG_CAPACITY;
        return false;
    }
        
    // Wipe out the target buffer
    std::memset(dst, 0, size);
    
    // Export all blocks
    for (isize i = 0; i < count; i++) {
        
        blocks[first + i]->exportBlock(dst + i * bsize, bsize);
    }

    debug(FS_DEBUG, "Success\n");

    if (err) *err = ERROR_OK;
    return true;
}

void
FSDevice::importDirectory(const string &path, bool recursive)
{
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) throw VAError(ERROR_FILE_CANT_READ);
    
    importDirectory(path, dir, recursive);
    closedir(dir);
}

void
FSDevice::importDirectory(const string &path, DIR *dir, bool recursive)
{
    assert(dir);
    
    struct dirent *item;
    
    while ((item = readdir(dir))) {

        // Skip '.', '..' and all hidden files
        if (item->d_name[0] == '.') continue;

        // Assemble file name
        string name = path + "/" + string(item->d_name);
        
        debug(FS_DEBUG, "Importing %s\n", name.c_str());
        
        if (item->d_type == DT_DIR) {
            
            // Add directory
            if(createDir(item->d_name) && recursive) {
                changeDir(item->d_name);
                importDirectory(name, recursive);
            }
            
        } else {
            
            // Add file
            u8 *buffer; isize size;
            if (util::loadFile(string(name), &buffer, &size)) {
                createFile(item->d_name, buffer, size);
                delete(buffer);
            }
        }        
    }
}

void
FSDevice::exportDirectory(const string &path)
{
    // Only proceed if path points to an empty directory
    long numItems = util::numDirectoryItems(path);
    if (numItems != 0) throw VAError(ERROR_FS_DIRECTORY_NOT_EMPTY);
    
    // Collect files and directories
    std::vector<Block> items;
    collect(cd, items);
    
    // Export all items
    for (auto const& i : items) {
        if (ErrorCode error = blockPtr(i)->exportBlock(path.c_str()); error != ERROR_OK) {
            throw VAError(error);
        }
    }
    
    msg("Exported %zu items", items.size());
}
