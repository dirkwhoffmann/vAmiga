// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSService.h"
#include <unordered_set>

namespace vamiga {

enum class TraversalOrder { DFS, BFS };

struct FSTree {

    BlockNr nr;
    std::vector<FSTree> children;

    FSTree() : nr(0) {}

    // Range helpers
    auto dfs() const { return Range<TraversalOrder::DFS>(*this); }
    auto bfs() const { return Range<TraversalOrder::BFS>(*this); }

private:

    template <TraversalOrder O>
    class Iterator {

        using NodePtr = const FSTree*;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = FSTree;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const FSTree*;
        using reference         = const FSTree&;

        Iterator() = default;
        explicit Iterator(const FSTree& root)
        {
            if constexpr (O == TraversalOrder::DFS) {
                stack.push_back(&root);
            } else {
                queue.push_back(&root);
            }
        }

        reference operator*() const { return *current(); }
        pointer operator->() const  { return current(); }

        Iterator& operator++()
        {
            advance();
            return *this;
        }

        bool operator==(const Iterator& other) const
        {
            return current() == other.current();
        }

        bool operator!=(const Iterator& other) const
        {
            return !(*this == other);
        }

    private:

        std::vector<NodePtr> stack; // DFS
        std::deque<NodePtr>  queue; // BFS

        NodePtr current() const
        {
            if constexpr (O == TraversalOrder::DFS) {
                return stack.empty() ? nullptr : stack.back();
            } else {
                return queue.empty() ? nullptr : queue.front();
            }
        }

        void advance()
        {
            if constexpr (O == TraversalOrder::DFS) {

                NodePtr node = stack.back();
                stack.pop_back();

                // Push children in reverse so leftmost is visited first
                for (auto it = node->children.rbegin();
                     it != node->children.rend(); ++it) {
                    stack.push_back(&*it);
                }

            } else {

                NodePtr node = queue.front();
                queue.pop_front();

                for (const auto& child : node->children) {
                    queue.push_back(&child);
                }
            }
        }
    };

    template <TraversalOrder O>
    struct Range {
        const FSTree& root;
        explicit Range(const FSTree& r) : root(r) {}

        auto begin() const { return Iterator<O>(root); }
        auto end()   const { return Iterator<O>(); }
    };
};

struct FSTreeBuildOptions {

    // Accept / reject a node
    std::function<bool(const FSBlock &)> accept = [](const FSBlock &) { return true; };
    // std::function<bool(BlockNr)> acceptRef = [](BlockNr) { return true; }

    // Sort siblings
    std::function<bool(const FSBlock&, const FSBlock&)> sort;

    // Recurse into subdirectories
    bool recursive = true;
};

class FSTreeBuilder {

public:
    static FSTree build(const FSBlock &root,
                        const FSTreeBuildOptions &opt = {});

private:

    static FSTree buildRec(const FSBlock &node,
                           const FSTreeBuildOptions &opt,
                           std::unordered_set<BlockNr> &visited);
};

class FSTreePrinter {

public:
    static void list(const FileSystem& fs,
                     const FSTree& tree,
                     std::ostream& os,
                     const FSOpt& opt = {});

    static void listRec(const FileSystem& fs,
                        const FSTree& tree,
                        std::ostream& os,
                        const FSOpt& opt);

    static void listItems(const FileSystem& fs,
                          const FSTree& tree,
                          std::ostream& os,
                          const FSOpt& opt);
};



class FSWalker final : public FSService {

};

}
