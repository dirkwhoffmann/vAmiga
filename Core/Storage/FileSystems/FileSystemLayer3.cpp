// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystem.h"
#include "utl/support.h"

namespace vamiga {

void
FileSystem::cd(BlockNr nr)
{
    current = nr;
}

void
FileSystem::cd(const FSName &name)
{
    cd(seek(pwd(), name));
}

void
FileSystem::cd(const string &path)
{
    cd(seek(pwd(), path));
}

bool
FileSystem::exists(BlockNr top, const fs::path &path) const
{
    return trySeek(top, path).has_value();
}

optional<BlockNr>
FileSystem::trySeek(BlockNr top, const FSName &name) const
{
    auto &root = fetch(rootBlock);
    if (!root.isRoot()) return {};

    std::unordered_set<BlockNr> visited;

    // Check for special tokens
    if (name == "/")  return rootBlock;
    if (name == "")   return fetch(rootBlock).nr;
    if (name == ".")  return fetch(rootBlock).nr;
    if (name == "..") return fetch(rootBlock).getParentDirRef();

    // TODO: USE SEARCHDIR
    // Only proceed if a hash table is present
    if (root.hasHashTable()) {

        // Compute the table position and read the item
        u32 hash = name.hashValue(traits.dos) % root.hashTableSize();
        u32 ref = root.getHashRef(hash);

        // Traverse the linked list until the item has been found
        while (ref && visited.find(ref) == visited.end())  {

            auto *block = tryFetch(ref, { FSBlockType::USERDIR, FSBlockType::FILEHEADER });
            if (block == nullptr) break;

            if (block->isNamed(name)) return block->nr;

            visited.insert(ref);
            ref = block->getNextHashRef();
        }
    }

    return { };
}

optional<BlockNr>
FileSystem::trySeek(BlockNr top, const fs::path &name) const
{
    return trySeek(top, FSName(name));
}

optional<BlockNr>
FileSystem::trySeek(BlockNr top, const string &name) const
{
    return trySeek(top, FSName(name));
}

BlockNr
FileSystem::seek(BlockNr top, const FSName &name) const
{
    if (auto it = trySeek(top, name)) return *it;
    throw FSError(FSError::FS_NOT_FOUND, name.cpp_str());
}

BlockNr
FileSystem::seek(BlockNr top, const fs::path &name) const
{
    if (auto it = trySeek(top, name)) return *it;
    throw FSError(FSError::FS_NOT_FOUND, name.string());
}

BlockNr
FileSystem::seek(BlockNr top, const string &name) const
{
    if (auto it = trySeek(top, name)) return *it;
    throw FSError(FSError::FS_NOT_FOUND, name);
}

FSTree
FileSystem::build(BlockNr root, const FSTreeBuildOptions &opt)
{
    return FSTreeBuilder::build(fetch(root), opt); 
}

std::vector<const FSBlock *>
FileSystem::find(const FSOpt &opt) const
{
    return find(fetch(pwd()), opt);
}

std::vector<const FSBlock *>
FileSystem::find(const FSBlock &root, const FSOpt &opt) const
{
    require.fileOrDirectory(root.nr);
    return find(&root, opt);
}

std::vector<const FSBlock *>
FileSystem::find(const FSBlock *root, const FSOpt &opt) const
{
    if (!root) return {};
    std::unordered_set<BlockNr> visited;
    return find(root, opt, visited);
}

std::vector<BlockNr>
FileSystem::find(BlockNr root, const FSOpt &opt) const
{
    return FSBlock::refs(find(tryFetch(root), opt));
}

std::vector<const FSBlock *>
FileSystem::find(const FSBlock *root, const FSOpt &opt, std::unordered_set<BlockNr> &visited) const
{
    std::vector<const FSBlock *> result;

    // Collect all items in the hash table
    auto hashedBlocks = collectHashedBlocks(*root);

    for (auto it = hashedBlocks.begin(); it != hashedBlocks.end(); it++) {

        // Add item if accepted
        if (opt.accept(*it)) result.push_back(*it);

        // Break the loop if this block has been visited before
        if (visited.contains((*it)->nr)) throw FSError(FSError::FS_HAS_CYCLES);

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

/*
std::vector<BlockNr>
FileSystem::find(BlockNr top, const FSOpt &opt, std::unordered_set<BlockNr> &visited) const
{
    std::vector<BlockNr> result;

    // Collect all items in the hash table
    auto hashedBlocks = collectHashedBlocks(top);

    for (auto it = hashedBlocks.begin(); it != hashedBlocks.end(); it++) {

        // Add item if accepted
        if (opt.accept(fetch(*it))) result.push_back(*it);

        // Break the loop if this block has been visited before
        if (visited.contains(*it)) throw FSError(FSError::FS_HAS_CYCLES);

        // Remember the block as visited
        visited.insert(*it);
    }

    // Search subdirectories
    if (opt.recursive) {

        for (auto &it : hashedBlocks) {

            if (fetch(it).isDirectory()) {

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
*/



std::vector<const FSBlock *>
FileSystem::find(const FSPattern &pattern) const
{
    std::vector<BlockNr> result;

    // Determine the directory to start searching
    auto &start = pattern.isAbsolute() ? fetch(root()) : fetch(pwd());

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

std::vector<BlockNr>
FileSystem::find(BlockNr root, const FSPattern &pattern) const
{
    return FSBlock::refs(find(tryFetch(root), pattern));
}

std::vector<const FSBlock *>
FileSystem::match(const FSPattern &pattern) const
{
    if (pattern.isAbsolute()) {
        return match(&fetch(root()), pattern.splitted());
    } else {
        return match(&fetch(pwd()), pattern.splitted());
    }
}

std::vector<const FSBlock *>
FileSystem::match(const FSBlock *node, const FSPattern &pattern) const
{
    if (pattern.isAbsolute()) {
        return match(&fetch(root()), pattern.splitted());
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
    auto items = OldFSTree(*root, { .recursive = false} );

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

std::vector<BlockNr>
FileSystem::match(BlockNr root, const FSPattern &pattern) const
{
    return FSBlock::refs(match(tryFetch(root), pattern));
}

}
