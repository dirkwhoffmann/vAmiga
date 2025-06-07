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


FSPath::FSPath(const FileSystem &fs, Block dir) : fs(fs), dir(dir)
{
    selfcheck();
}

void
FSPath::selfcheck()
{
    if (!isRoot() && !isFile() && !isDirectory()) throw AppError(Fault::FS_INVALID_BLOCK_TYPE);
}

bool
FSPath::isRoot()
{
    return fs.blockType(dir) == FSBlockType::ROOT_BLOCK;
}

bool
FSPath::isFile()
{
    return fs.blockType(dir) == FSBlockType::FILEHEADER_BLOCK;
}

bool
FSPath::isDirectory()
{
    return fs.blockType(dir) == FSBlockType::USERDIR_BLOCK;
}

Block
FSPath::seekRef(FSName name) const
{
    std::set<Block> visited;

    // Only proceed if a hash table is present
    FSBlock *cdb = fs.blockPtr(dir);
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

void
FSPath::cd(FSName name)
{
    dir = seekRef(name);
    selfcheck();
}

void
FSPath::parent()
{
    if (!isRoot()) {

        dir = fs.blockPtr(dir)->getParentDirRef();
        selfcheck();
    }
}

FSPath&
FSPath::operator/= (const FSName &name)
{
    cd(name);
    return *this;
}

FSPath
FSPath::operator/ (const FSName &name) const
{
    FSPath result = *this;
    result /= name;
    return result;
}

fs::path
FSPath::getPath() const
{
    fs::path result;
    std::set<Block> visited;

    auto block = fs.blockPtr(dir);

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


}
