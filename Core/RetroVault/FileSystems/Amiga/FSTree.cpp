// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSTree.h"
#include "FileSystem.h"
#include <algorithm>

namespace retro::amigafs {

FSTree
FSTreeBuilder::build(const FSBlock &root,
                     const FSTreeBuildOptions &opt)
{
    std::unordered_set<BlockNr> visited;
    return buildRec(root, opt, 0, visited);
}

FSTree
FSTreeBuilder::buildRec(const FSBlock &node,
                        const FSTreeBuildOptions &opt,
                        isize depth,
                        std::unordered_set<BlockNr> &visited)
{
    auto& fs = *node.fs;

    // Check for cycles
    if (!visited.insert(node.nr).second) throw FSError(FSError::FS_HAS_CYCLES);

    // Create a tree for the top-level node
    FSTree tree { .nr = node.nr };

    if (node.isDirectory() && depth < opt.depth) {

        // Collect
        auto children = fs.collectHashedBlocks(node.nr);

        // Filter
        std::vector<const FSBlock *> accepted;
        for (auto &ref : children) {
            auto &child = fs.fetch(ref);
            if (opt.accept(child)) accepted.push_back(&child);
        }

        // Sort
        if (opt.sort) {
            std::sort(accepted.begin(), accepted.end(), [&](auto *a, auto *b) {
                return opt.sort(*a, *b);
            });
        }

        // Recurse
        for (auto *child : accepted) {
            tree.children.push_back(buildRec(*child, opt, depth + 1, visited));
        }
    }

    return tree;
}

}
