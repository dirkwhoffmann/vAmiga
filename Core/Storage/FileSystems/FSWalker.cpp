// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSWalker.h"
#include "FileSystem.h"

namespace vamiga {

FSTree
FSTreeBuilder::build(const FSBlock &root, const FSTreeBuildOptions &opt)
{
    std::unordered_set<BlockNr> visited;
    return buildRec(root, opt, 0, visited);
}

FSTree
FSTreeBuilder::buildRec(const FSBlock &node, const FSTreeBuildOptions &opt,
                        isize depth, std::unordered_set<BlockNr> &visited)
{
    auto& fs = *node.fs;

    // Ensure the node is a user directory block or a file header block
    // fs.require.fileOrDirectory(node.nr);

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

void
FSTreePrinter::list(const FileSystem& fs,
                    const FSTree& tree,
                    std::ostream& os,
                    const FSOpt& opt)
{
    FSOpt options = opt;

    if (!options.formatter) {
        options.formatter = [](const FSBlock& node) {
            return node.cppName() +
                   (node.isDirectory() ? " (dir)" : "\t");
        };
    }

    listRec(fs, tree, os, options);
}

void
FSTreePrinter::listRec(const FileSystem& fs,
                       const FSTree& tree,
                       std::ostream& os,
                       const FSOpt& opt)
{
    const FSBlock& node = fs.fetch(tree.nr);

    if (opt.recursive) {

        // Header
        os << "Directory " << node.absName()
           << ":" << std::endl << std::endl;

        listItems(fs, tree, os, opt);

        for (const auto& child : tree.children) {
            const FSBlock& cb = fs.fetch(child.nr);
            if (cb.isDirectory()) {
                os << std::endl;
                listRec(fs, child, os, opt);
            }
        }

    } else {
        listItems(fs, tree, os, opt);
    }
}

void
FSTreePrinter::listItems(const FileSystem& fs,
                         const FSTree& tree,
                         std::ostream& os,
                         const FSOpt& opt)
{
    std::vector<std::string> strs;

    for (const auto& child : tree.children) {
        const FSBlock& node = fs.fetch(child.nr);
        if (opt.accept(node)) {
            strs.push_back(opt.formatter(node));
        }
    }

    if (strs.empty()) return;

    int tab = 0;
    for (auto& s : strs) {
        tab = std::max(tab, int(s.length()));
    }

    isize column = 0;

    for (auto& item : strs) {

        if (!item.empty() && item.back() == '\t') {

            item.pop_back();
            if (column == 0)
                os << std::string(opt.indent, ' ');

            os << std::left
               << std::setw(std::max(tab, 35))
               << item;

            if (column++ > 0) {
                os << std::endl;
                column = 0;
            }

        } else {

            if (column == 0)
                os << std::string(opt.indent, ' ');

            if (column > 0) {
                os << std::endl;
                column = 0;
            }

            os << std::left
               << std::setw(std::max(tab, 35))
               << item
               << std::endl;
        }
    }

    if (column) os << std::endl;
}

}
