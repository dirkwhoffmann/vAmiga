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

bool
FileSystem::exists(const fs::path &path) const
{
    return trySeek(path).has_value();
}

optional<BlockNr>
FileSystem::trySeek(const fs::path &path) const
{
    BlockNr current = pwd();

    printf("Path '%s' -> ", path.c_str());
    for (const auto &component : path) { printf("'%s' ", component.c_str()); }
    printf("\n");

    for (const auto &component : path) {

        printf("Seeking '%s' in '%s'\n", component.c_str(), fetch(current).absName().c_str());

        // Check for special tokens
        if (component == "/" ) { current = rootBlock; continue; }
        if (component == ""  ) { continue; }
        if (component == "." ) { continue; }
        if (component == "..") { current = fetch(current).getParentDirRef(); continue; }

        auto next = searchdir(current, FSName(component));
        if (!next) { printf("%s not found", FSName(component).c_str());  return { }; }

        printf("Found %d\n", *next);

        current = *next;
    }

    return current;
}

/*
vector<BlockNr>
FileSystem::trySeek(const FSPattern &pattern) const
{
    auto tree = build(top, {

        .accept = accept::pattern(pattern),
        .sort   = sort::none,
        .depth  = 1
    });

    vector<BlockNr> result;
    result.reserve(tree.children.size());

    for (const auto &child : tree.children) {
        result.push_back(child.nr);
    }
    return result;
}
*/

BlockNr
FileSystem::seek(const fs::path &name) const
{
    if (auto it = trySeek(name)) return *it;
    throw FSError(FSError::FS_NOT_FOUND, name.string());
}

FSTree
FileSystem::build(BlockNr root, const FSTreeBuildOptions &opt) const
{
    return FSTreeBuilder::build(fetch(root), opt); 
}

vector<const FSBlock *>
FileSystem::newMatch(BlockNr top, const FSPattern &pattern) const
{
    if (pattern.isAbsolute()) {
        return newMatch(&fetch(root()), pattern.splitted());
    } else {
        return newMatch(&fetch(pwd()), pattern.splitted());
    }
}

vector<const FSBlock *>
FileSystem::newMatch(const FSBlock *top, vector<FSPattern> patterns) const
{
    std::vector<const FSBlock *> result;

    if (patterns.empty()) return {};

    // Get all directory items
    auto items = build(top->nr, { .accept = accept::all, .sort = sort::none, .depth = 1 });

    // Extract the first pattern
    auto pattern = patterns.front(); patterns.erase(patterns.begin());

    if (patterns.empty()) {

        // Collect all matching items
        for (auto &item : items.children) {

            auto &node = fetch(item.nr);
            if (pattern.match(node.cppName())) {
                result.push_back(&node);
            }
        }

    } else {

        // Continue by searching all matching subdirectories
        for (auto &item : items.children) {

            auto &node = fetch(item.nr);
            if (node.isDirectory() && pattern.match(node.cppName())) {
                auto subdirItems = newMatch(&node, patterns);
                result.insert(result.end(), subdirItems.begin(), subdirItems.end());
            }
        }
    }

    return result;
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
