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
    const class FileSystem &fs;

    // The referenced block
    Block ref;

    // Constructors
    FSPath(const FSPath &);
    FSPath(const FileSystem &fs);
    FSPath(const FileSystem &fs, Block dir);
    FSPath(const FileSystem &fs, struct FSBlock *dir);

    // Throws an exception if the object is invalid
    void selfcheck() const;

    // Operator overloads
    FSPath &operator=(const FSPath &);
    FSPath& operator/=(const FSName &name);
    FSPath operator/(const FSName &name) const;

    // Informs about where this path points to
    bool isRoot() const;
    bool isFile() const;
    bool isDirectory() const;


    //
    // Representing the path
    //

    // Returns a pointer to the corresponding FSBlock
    FSBlock *ptr() const;

    // Returns the last path component
    FSName last() const;

    // Returns a string representation of this path
    string name() const;

    // Converts the path to a host path
    fs::path getPath() const;

    // Represents the path as a collections of block nodes, starting from the root
    std::vector<Block> refs() const;


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

}
