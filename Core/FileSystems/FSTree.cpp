// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FSTree.h"

namespace vamiga {

void
FSTree::dfsWalk(std::function<void(const FSTree &)> func)
{
    func(*this);
    for (auto &it : children) it.dfsWalk(func);
}

void
FSTree::bfsWalk(std::function<void(const FSTree &)> func)
{
    func(*this);
    bfsWalkRec(func);
}

void
FSTree::bfsWalkRec(std::function<void(const FSTree &)> func)
{
    for (auto &it : children) { func(it); }
    for (auto &it : children) { if (!it.children.empty()) it.bfsWalkRec(func); }
}

void
FSTree::sort(std::function<bool(FSBlock &,FSBlock &)> sort)
{
    if (sort == nullptr) return;

    auto comperator = [sort](const FSTree &b1, const FSTree &b2) { return sort(*b1.node, *b2.node); };
    std::sort(children.begin(), children.end(), comperator);
}

}
