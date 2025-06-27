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
#include "FSBlock.h"

namespace vamiga {

struct FSNode
{
    const class FileSystem *fs = nullptr;

    // The referenced block
    Block ref;

    // Constructors
    FSNode(const FileSystem *fs);
    FSNode(const FileSystem *fs, Block dir);
    FSNode(const FileSystem *fs, struct FSBlock *dir);

    // Operator overloads
    FSNode &operator=(const FSNode &);
    FSNode &operator/=(const FSName &name);
    FSNode operator/(const FSName &name) const;

    // Informs about where this path points to
    bool isRoot() const;
    bool isFile() const;
    bool isDirectory() const;
    bool isRegular() const;
    bool isHashable() const;

    // Checks if the path matches a search pattern
    bool matches(const FSPattern &pattern) const;


    //
    // Representing the path
    //

    // Returns a pointer to the corresponding FSBlock
    FSBlock *ptr() const;

    // Returns a string representation of this path
    FSName last() const;
    string absName() const;
    string relName(const FSNode &root) const;
    string relName() const;

    // Converts the path to a host path
    fs::path getPath() const;

    // Represents the path as a collections of block nodes, starting from 'root'
    std::vector<Block> refs(Block root = 0) const;


    //
    // Querying block properties
    //

    string getProtectionBitString() const;


    //
    // Traversing the directory tree
    //

    // Returns the parent directory
    FSNode parent() const;

    // Seeks a file or directory
    FSNode seek(const FSName &name) const;
    FSNode seek(const FSString &name) const;
    FSNode seek(const std::vector<FSName> &name) const;
    FSNode seek(const std::vector<string> &name) const;
    FSNode seek(const fs::path &name) const;
    FSNode seek(const string &name) const;
    FSNode seek(const char *name) const;

    FSNode seekDir(const FSName &dir) const;
    FSNode seekDir(const FSString &dir) const;
    FSNode seekDir(const std::vector<FSName> &dir) const;
    FSNode seekDir(const std::vector<string> &dir) const;
    FSNode seekDir(const fs::path &dir) const;
    FSNode seekDir(const string &dir) const;
    FSNode seekDir(const char *dir) const;

    FSNode seekFile(const FSName &file) const;
    FSNode seekFile(const FSString &file) const;
    FSNode seekFile(const std::vector<FSName> &file) const;
    FSNode seekFile(const std::vector<string> &file) const;
    FSNode seekFile(const fs::path &file) const;
    FSNode seekFile(const string &file) const;
    FSNode seekFile(const char *file) const;

    // Moves up or down in the directory tree
    void cd(const FSName &name) { ref = seekDir(name).ref; }
    void cd(const FSString &name) { ref = seekDir(name).ref; }
    void cd(const std::vector<FSName> &name) { ref = seekDir(name).ref; }
    void cd(const std::vector<string> &name) { ref = seekDir(name).ref; }
    void cd(const fs::path &name) { ref = seekDir(name).ref; }
    void cd(const string &name) { ref = seekDir(name).ref; }
    void cd(const char *name) { ref = seekDir(name).ref; }

    // Returns a collection of paths to all items in a directory
    std::vector<FSNode> collect(const FSOpt &opt = {}) const;
    std::vector<FSNode> collectDirs(const FSOpt &opt = {}) const;
    std::vector<FSNode> collectFiles(const FSOpt &opt = {}) const;

    friend std::ostream &operator<<(std::ostream &os, const FSNode &str);
};

//
// Comparison function used for sorting
//

namespace sort {

inline std::function<bool(const FSBlock &, const FSBlock &)> dafa = [](const FSBlock &b1, const FSBlock &b2) {

    if ( b1.isDirectory() && !b2.isDirectory()) return true;
    if (!b1.isDirectory() &&  b2.isDirectory()) return false;
    return b1.pathName() < b2.pathName();
};

inline std::function<bool(const FSBlock &, const FSBlock &)> alpha = [](const FSBlock &b1, const FSBlock &b2) {

    return b1.pathName() < b2.pathName();
};

inline std::function<bool(const FSBlock &, const FSBlock &)> none = nullptr;

}

}
