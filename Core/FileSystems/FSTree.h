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

    void init(const FSBlock &path, const FSOpt &opt, std::unordered_set<Block> &visited);

    bool empty() { return node == nullptr; }
    isize size();

    void addChild(const FSBlock *node) { if (node) children.push_back(FSTree(node)); }

    // Traverses the tree and applies a function at each node
    void bfsWalk(std::function<void(const FSTree &)>);
    void dfsWalk(std::function<void(const FSTree &)>);

    void sort(std::function<bool(const FSBlock &,const FSBlock &)>);

private:

    void bfsWalkRec(std::function<void(const FSTree &)>);
};

}
