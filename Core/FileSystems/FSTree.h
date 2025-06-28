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

namespace vamiga {

struct FSTree {

    FSBlock *node;
    std::vector<FSTree> children;

    FSTree(FSBlock *node) : node(node) {}

    bool empty() { return node == nullptr; }

    void addChild(FSBlock *node) { if (node) children.push_back(FSTree(node)); }

    // Traverses the tree and applies a function at each node
    void bfsWalk(std::function<void(const FSTree &)>);
    void dfsWalk(std::function<void(const FSTree &)>);

    void sort(std::function<bool(FSBlock &,FSBlock &)>);

private:

    void bfsWalkRec(std::function<void(const FSTree &)>);
};

}
