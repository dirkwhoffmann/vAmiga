// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FSPath.h"
#include "FileSystem.h"

namespace vamiga {

FSPath::FSPath(const FSPath &path) : fs(path.fs), ref(path.ref)
{

}

FSPath::FSPath(const FileSystem &fs, Block dir) : fs(fs), ref(dir)
{
    selfcheck();
}

FSPath::FSPath(const FileSystem &fs, FSBlock *dir) : FSPath(fs, dir->nr)
{

}

FSPath::FSPath(const FileSystem &fs, const fs::path &path) : FSPath(seek(path))
{

}

void
FSPath::selfcheck() const
{
    // Check if the block number is in the valid range
    if (!ptr()) throw AppError(Fault::FS_INVALID_BLOCK_TYPE);

    // Check the block type
    if (!isRoot() && !isFile() && !isDirectory()) throw AppError(Fault::FS_INVALID_BLOCK_TYPE);
}

FSPath&
FSPath::operator=(const FSPath &path)
{
    ref = path.ref;
    return *this;
}

FSPath&
FSPath::operator/=(const FSName &name)
{
    cd(name);
    return *this;
}

FSPath
FSPath::operator/(const FSName &name) const
{
    FSPath result = *this;
    result /= name;
    return result;
}

bool
FSPath::isRoot() const
{
    return fs.blockType(ref) == FSBlockType::ROOT_BLOCK;
}

bool
FSPath::isFile() const
{
    return fs.blockType(ref) == FSBlockType::FILEHEADER_BLOCK;
}

bool
FSPath::isDirectory() const
{
    return isRoot() || fs.blockType(ref) == FSBlockType::USERDIR_BLOCK;
}

FSBlock *
FSPath::ptr() const
{
    return fs.blockPtr(ref);
}

fs::path
FSPath::getPath() const
{
    fs::path result;
    std::set<Block> visited;

    auto block = fs.blockPtr(ref);

    while (block) {

        // Break the loop if this block has an invalid type
        if (!fs.hashableBlockPtr(block->nr)) break;

        // Break the loop if this block was visited before
        if (visited.find(block->nr) != visited.end()) break;

        // Add the block to the set of visited blocks
        visited.insert(block->nr);

        // Expand the path
        auto name = block->getName().path();
        result = result.empty() ? name : name / result;

        // Continue with the parent block
        block = block->getParentDirBlock();
    }

    return result;
}

Block
FSPath::seek(const FSName &name) const
{
    std::set<Block> visited;

    // Only proceed if a hash table is present
    FSBlock *cdb = fs.blockPtr(ref);
    if (!cdb || cdb->hashTableSize() == 0) return 0;

    // Compute the table position and read the item
    u32 hash = name.hashValue() % cdb->hashTableSize();
    u32 ref = cdb->getHashRef(hash);

    // Traverse the linked list until the item has been found
    while (ref && visited.find(ref) == visited.end())  {

        FSBlock *item = fs.hashableBlockPtr(ref);
        if (item == nullptr) break;

        if (item->isNamed(name)) return item->nr;

        visited.insert(ref);
        ref = item->getNextHashRef();
    }

    return 0;
}

FSPath
FSPath::seekDir(const FSName &name) const
{
    if (auto result = FSPath(fs, seek(name)); result.isDirectory()) return result;
    throw AppError(Fault::DIR_NOT_FOUND);
}

FSPath
FSPath::seekFile(const FSName &name) const
{
    if (auto result = FSPath(fs, seek(name)); result.isFile()) return result;
    throw AppError(Fault::FILE_NOT_FOUND);
}

FSPath
FSPath::seek(const fs::path &path) const
{
    FSPath result = fs.rootDir();

    for (const auto& part : path) {

        if (part == path.filename()) {
            result = result.seekFile(FSName(part));
        } else {
            result = result.seekDir(FSName(part));
        }
    }
    return result;
}

FSPath
FSPath::seekDir(const fs::path &path) const
{
    if (FSPath result = seek(path); result.isDirectory()) {
        return result;
    }
    throw AppError(Fault::DIR_NOT_FOUND);
}

FSPath
FSPath::seekFile(const fs::path &path) const
{
    if (FSPath result = seek(path); result.isFile()) {
        return result;
    }
    throw AppError(Fault::DIR_NOT_FOUND);
}

void
FSPath::cd(FSName name)
{
    ref = seek(name);
    selfcheck();
}

void
FSPath::cd(const fs::path &path)
{
    if (!fs::is_directory(path)) return;

    for (const auto& part : path) {
        cd(FSName(part));
    }
}

void
FSPath::parent()
{
    if (!isRoot()) {

        ref = fs.blockPtr(ref)->getParentDirRef();
        selfcheck();
    }
}

}
