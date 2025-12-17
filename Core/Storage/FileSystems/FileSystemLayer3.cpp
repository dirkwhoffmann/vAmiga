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

void
FileSystem::cd(const FSName &name)
{
    if (auto ptr = seekPtr(&pwd(), name); ptr) cd (*ptr);
    throw FSError(fault::FS_NOT_FOUND, name.cpp_str());
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
    throw FSError(fault::FS_NOT_FOUND, path);
}

bool
FileSystem::exists(const FSBlock &top, const fs::path &path) const
{
    return seekPtr(&top, path) != nullptr;
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

    // TODO: USE SEARCHDIR
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

    auto parts = utl::split(name, '/');
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
    throw FSError(fault::FS_NOT_FOUND, name.cpp_str());
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
    throw FSError(fault::FS_NOT_FOUND, name.string());
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
    throw FSError(fault::FS_NOT_FOUND, name);
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
    require::fileOrDirectory(root);
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
        if (visited.contains((*it)->nr)) throw FSError(fault::FS_HAS_CYCLES);

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

}
