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

struct FSPath
{
    const class FileSystem *fs = nullptr;

    // The referenced block
    Block ref;

    // Constructors
    FSPath(const FileSystem *fs);
    FSPath(const FileSystem *fs, Block dir);
    FSPath(const FileSystem *fs, struct FSBlock *dir);

    // Operator overloads
    FSPath &operator=(const FSPath &);
    FSPath &operator/=(const FSName &name);
    FSPath operator/(const FSName &name) const;

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
    FSName last() const; // RENAME TO name()
    string absName() const;
    string relName(const FSPath &root) const;
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
    FSPath parent() const;

    // Seeks a file or directory
    FSPath seek(const FSName &name) const;
    FSPath seek(const FSString &name) const;
    FSPath seek(const std::vector<FSName> &name) const;
    FSPath seek(const std::vector<string> &name) const;
    FSPath seek(const fs::path &name) const;
    FSPath seek(const string &name) const;
    FSPath seek(const char *name) const;

    FSPath seekDir(const FSName &dir) const;
    FSPath seekDir(const FSString &dir) const;
    FSPath seekDir(const std::vector<FSName> &dir) const;
    FSPath seekDir(const std::vector<string> &dir) const;
    FSPath seekDir(const fs::path &dir) const;
    FSPath seekDir(const string &dir) const;
    FSPath seekDir(const char *dir) const;

    FSPath seekFile(const FSName &file) const;
    FSPath seekFile(const FSString &file) const;
    FSPath seekFile(const std::vector<FSName> &file) const;
    FSPath seekFile(const std::vector<string> &file) const;
    FSPath seekFile(const fs::path &file) const;
    FSPath seekFile(const string &file) const;
    FSPath seekFile(const char *file) const;

    // Moves up or down in the directory tree
    void cd(const FSName &name) { ref = seekDir(name).ref; }
    void cd(const FSString &name) { ref = seekDir(name).ref; }
    void cd(const std::vector<FSName> &name) { ref = seekDir(name).ref; }
    void cd(const std::vector<string> &name) { ref = seekDir(name).ref; }
    void cd(const fs::path &name) { ref = seekDir(name).ref; }
    void cd(const string &name) { ref = seekDir(name).ref; }
    void cd(const char *name) { ref = seekDir(name).ref; }

    // Returns a collection of paths to all items in a directory
    std::vector<FSPath> collect(const FSOpt &opt = {}) const;
    std::vector<FSPath> collectDirs(const FSOpt &opt = {}) const;
    std::vector<FSPath> collectFiles(const FSOpt &opt = {}) const;

    friend std::ostream &operator<<(std::ostream &os, const FSPath &str);
};

//
// Comparison function used for sorting
//

namespace sort {

inline std::function<bool(const FSPath &, const FSPath &)> dafa = [](const FSPath &a, const FSPath &b) {

    if ( a.isDirectory() && !b.isDirectory()) return true;
    if (!a.isDirectory() &&  b.isDirectory()) return false;
    return a.last() < b.last();
};

inline std::function<bool(const FSPath &, const FSPath &)> alpha = [](const FSPath &a, const FSPath &b) {

    return a.last() < b.last();
};

inline std::function<bool(const FSPath &, const FSPath &)> none = nullptr;

}

}
