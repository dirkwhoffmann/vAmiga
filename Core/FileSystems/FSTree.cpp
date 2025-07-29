// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FSTree.h"
#include "FileSystem.h"
#include "Host.h"

namespace vamiga {

FSTree::FSTree(const FSBlock &top, const FSOpt &opt)
{
    std::unordered_set<Block> visited;
    init(top, opt, visited);
};

FSTree::FSTree(const std::vector<const FSBlock *> nodes, const FSOpt &opt) : node(nullptr)
{
    for (auto &it : nodes) addChild(it);
    sort(opt.sort);
}

void
FSTree::init(const FSBlock &top, const FSOpt &opt, std::unordered_set<Block> &visited)
{
    auto &fs = *top.fs;

    top.fs->require_file_or_directory(top);
    node = fs.read(top.nr);

    // Collect all items in the hash table
    auto hashedBlocks = fs.collectHashedBlocks(top);

    for (auto it = hashedBlocks.begin(); it != hashedBlocks.end(); it++) {

        // Add item to the tree
        if (opt.accept(*it)) children.push_back(*it);

        // Break the loop if this block has been visited before
        if (visited.contains((*it)->nr)) throw AppError(Fault::FS_HAS_CYCLES);

        // Remember the block as visited
        visited.insert((*it)->nr);
    }

    // Sort the items
    sort(opt.sort);

    // Add subdirectories if requested
    if (opt.recursive) {
        for (auto &it : children) {
            if (it.node->isDirectory()) { it.init(*it.node, opt, visited); }
        }
    }
}

isize
FSTree::size() const
{
    isize result = 1;
    for (auto &it : children) result += it.size();
    return result;
}

fs::path
FSTree::hostName() const
{
    if (!node) return {};
    return Host::sanitize(node->name().cpp_str());
}


void
FSTree::dfsWalk(std::function<void(const FSTree &)> func)
{
    if (!empty()) {

        func(*this);
        for (auto &it : children) it.dfsWalk(func);
    }
}

void
FSTree::bfsWalk(std::function<void(const FSTree &)> func)
{
    if (!empty()) {

        func(*this);
        bfsWalkRec(func);
    }
}

void
FSTree::bfsWalkRec(std::function<void(const FSTree &)> func)
{
    if (!empty()) {
        
        for (auto &it : children) { func(it); }
        for (auto &it : children) { if (!it.children.empty()) it.bfsWalkRec(func); }
    }
}

void
FSTree::sort(std::function<bool(const FSBlock &,const FSBlock &)> sort)
{
    if (sort == nullptr) return;

    auto comperator = [sort](const FSTree &b1, const FSTree &b2) { return sort(*b1.node, *b2.node); };
    std::sort(children.begin(), children.end(), comperator);
}

void
FSTree::list(std::ostream &os, const FSOpt &opt) const
{
    auto options = opt;

    if (!options.formatter) {

        // Assign a default formatter as none is given
        options.formatter = [&](const FSBlock &node) {
            return node.cppName() + (node.isDirectory() ? " (dir)" : "\t");
        };
    }
    listRec(os, options);
}

void
FSTree::listRec(std::ostream &os, const FSOpt &opt) const
{
    if (opt.recursive) {

        // Print header
        os << "Directory " << node->absName() << ":" << std::endl << std::endl;

        // Print all directoy items
        listItems(os, opt);

        // Print all non-empty subdirectories
        for (auto &it : children) {
            if (it.isDirectory()) { os << std::endl; it.listRec(os, opt); }
        }

    } else {

        // Print all directoy items
        listItems(os, opt);
    }
}

void
FSTree::listItems(std::ostream &os, const FSOpt &opt) const
{
    // Collect all displayed strings
    std::vector<string> strs;
    for (auto &it : children) { if (opt.accept(it.node)) strs.push_back(opt.formatter(*it.node)); }
    if (strs.empty()) return;

    // Determine the longest entry
    int tab = 0; for (auto &it: strs) tab = std::max(tab, int(it.length()));

    // List all items
    isize column = 0; for (auto &item : strs) {

        // Print in two columns if the name ends with a tab character
        if (item.back() == '\t') {

            item.pop_back();
            if (column == 0) os << string(opt.indent, ' ');
            os << std::left << std::setw(std::max(tab, 35)) << item;
            if (column++ > 0) { os << std::endl; column = 0; }

        } else {

            if (column == 0) os << string(opt.indent, ' ');
            if (column > 0) { os << std::endl; column = 0; }
            os << std::left << std::setw(std::max(tab, 35)) << item << std::endl;
        }
    }

    if (column) os << std::endl;
}

void
FSTree::save(const fs::path &path, const FSOpt &opt) const
{
    if (isDirectory()) {

        if (fs::exists(path)) {

            if (!fs::is_directory(path)) {
                throw AppError(Fault::FS_NOT_A_DIRECTORY, path.string());
            }
            if (!fs::is_empty(path)) {
                throw AppError(Fault::FS_DIR_NOT_EMPTY, path.string());
            }

        } else {

            fs::create_directories(path);
        }
        saveDir(path, opt);
    }

    if (isFile()) {

        if (fs::exists(path)) {
            throw AppError(Fault::FS_EXISTS, path.string());
        }
        saveFile(path, opt);
    }
}

void
FSTree::saveDir(const fs::path &path, const FSOpt &opt) const
{
    // Save files
    for (auto &it : children) {
        if (it.isFile()) it.node->exportBlock(path / it.hostName());
    }

    // Save directories
    if (opt.recursive) {
        for (auto &it : children) {
            if (it.isDirectory()) it.save(path / it.hostName(), opt);
        }
    }
}

void
FSTree::saveFile(const fs::path &path, const FSOpt &opt) const
{
    // Get data
    Buffer<u8> buffer; node->extractData(buffer);

    // Open file
    std::ofstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        throw AppError(Fault::FILE_CANT_CREATE, path);
    }

    // Write data
    stream.write((const char *)buffer.ptr, buffer.size);
    if (!stream) {
        throw AppError(Fault::FILE_CANT_WRITE, path);
    }
}

}
