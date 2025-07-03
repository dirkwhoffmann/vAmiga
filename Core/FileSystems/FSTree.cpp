// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FSTree.h"
#include "FileSystem.h"

namespace vamiga {

FSTree::FSTree(const FSBlock &path, const FSOpt &opt)
{
    std::unordered_set<Block> visited;
    init(path, opt, visited);
};

void
FSTree::init(const FSBlock &path, const FSOpt &opt, std::unordered_set<Block> &visited)
{
    auto &fs = *path.fs;

    path.fs->require_file_or_directory(path);
    node = fs.read(path.nr);

    // Collect all items in the hash table
    auto hashedBlocks = fs.collectHashedBlocks(path);

    for (auto it = hashedBlocks.begin(); it != hashedBlocks.end(); it++) {

        // Add item to the tree
        if (opt.accept(*it)) children.push_back(*it);

        // Break the loop if this block has been visited before
        if (visited.contains((*it)->nr)) throw AppError(Fault::FS_HAS_CYCLES);

        // Remember the block as visited
        visited.insert((*it)->nr);
    }

    // Sort the items
    sort(opt.sort);

    // Add subdirectories if requested
    if (opt.recursive) {
        for (auto &it : children) {
            if (it.node->isDirectory()) { it.init(*it.node, opt, visited); }
        }
    }
}

isize
FSTree::size()
{
    isize result = 1;
    for (auto &it : children) result += it.size();
    return result;
}

void
FSTree::dfsWalk(std::function<void(const FSTree &)> func)
{
    if (!empty()) {

        func(*this);
        for (auto &it : children) it.dfsWalk(func);
    }
}

void
FSTree::bfsWalk(std::function<void(const FSTree &)> func)
{
    if (!empty()) {

        func(*this);
        bfsWalkRec(func);
    }
}

void
FSTree::bfsWalkRec(std::function<void(const FSTree &)> func)
{
    if (!empty()) {
        
        for (auto &it : children) { func(it); }
        for (auto &it : children) { if (!it.children.empty()) it.bfsWalkRec(func); }
    }
}

void
FSTree::sort(std::function<bool(FSBlock &,FSBlock &)> sort)
{
    if (sort == nullptr) return;

    auto comperator = [sort](const FSTree &b1, const FSTree &b2) { return sort(*b1.node, *b2.node); };
    std::sort(children.begin(), children.end(), comperator);
}

}
