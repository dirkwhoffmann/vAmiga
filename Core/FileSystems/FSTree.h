// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "FSBlock.h"
#include <functional>
#include <unordered_set>

namespace vamiga {

struct FSTree {

    const FSBlock *node;
    std::vector<FSTree> children;

    FSTree() : node(nullptr) {}
    FSTree(const FSBlock *node) : node(node) {}
    FSTree(const FSBlock &path, const FSOpt &opt);
    FSTree(const std::vector<const FSBlock *> nodes, const FSOpt &opt);

    void init(const FSBlock &path, const FSOpt &opt, std::unordered_set<Block> &visited);

    bool empty() const { return node == nullptr; }
    isize size() const;

    // Add a child
    void addChild(const FSBlock *node) { if (node) children.push_back(FSTree(node)); }

    // Analyzes the node
    bool isFile() const { return node->isFile(); }
    bool isDirectory() const { return node->isDirectory(); }

    // Converts the node's name to a name compatible with the host file system
    fs::path hostName() const;

    // Traverses the tree and applies a function at each node
    void bfsWalk(std::function<void(const FSTree &)>);
    void bfsWalkRec(std::function<void(const FSTree &)>);
    void dfsWalk(std::function<void(const FSTree &)>);

    // Sorts the children using a custom comparator
    void sort(std::function<bool(const FSBlock &,const FSBlock &)>);

    // Pretty-prints the tree ('dir' command, 'list' command)
    void list(std::ostream &os, const FSOpt &opt = {}) const;

    // Exports the tree to the host file system
    void save(const fs::path &path, const FSOpt &opt = {}) const;

private:

    void listRec(std::ostream &os, const FSOpt &opt) const;
    void listItems(std::ostream &os, const FSOpt &opt) const;

    void saveFile(const fs::path &path, const FSOpt &opt = {}) const;
    void saveDir(const fs::path &path, const FSOpt &opt = {}) const;

};

}
