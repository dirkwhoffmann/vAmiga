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
#include "FSObjects.h"
#include <functional>

namespace vamiga {

struct FSTree {

    Block node;
    std::vector<FSTree> children;

    FSTree(Block node) : node(node) {}

    // Traverses the tree and applies a function at each node
    void bfsWalk(std::function<void(Block)>);
    void dfsWalk(std::function<void(Block)>);

    void sort(std::function<bool(Block,Block)>);

private:

    void bfsWalkRec(std::function<void(Block)>);
};

}
