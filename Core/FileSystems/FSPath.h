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

    // Seeks a file or directory
    FSPath seek(const FSName &name) const;
    FSPath seekDir(const FSName &name) const;
    FSPath seekFile(const FSName &name) const;

    FSPath seek(const FSString &name) const;
    FSPath seekDir(const FSString &name) const;
    FSPath seekFile(const FSString &name) const;

    FSPath seek(const std::vector<FSName> &path) const;
    FSPath seekDir(const std::vector<FSName> &path) const;
    FSPath seekFile(const std::vector<FSName> &path) const;

    FSPath seek(const std::vector<string> &path) const;
    FSPath seekDir(const std::vector<string> &path) const;
    FSPath seekFile(const std::vector<string> &path) const;

    FSPath seek(const fs::path &path) const;
    FSPath seekDir(const fs::path &path) const;
    FSPath seekFile(const fs::path &path) const;

    FSPath seek(const string &path) const;
    FSPath seekDir(const string &path) const;
    FSPath seekFile(const string &path) const;

    // Moves up or down in the directory tree
    FSPath cd(FSName name);
    FSPath cd(const std::vector<FSName> &names);
    FSPath cd(const std::vector<string> &names);
    FSPath cd(const string &path);
    FSPath parent();

    // Returns a collection of paths to all items in a directory
    // std::vector<FSPath> collect(bool recursive = false, bool sort = true, FSPathFilter filter = nullptr) const;
    std::vector<FSPath> collect(const FSOpt &opt = {}) const;

    friend std::ostream &operator<<(std::ostream &os, const FSPath &str);
};

}
