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
    FSPath(const FileSystem &fs, Block dir);
    FSPath(const FileSystem &fs, struct FSBlock *dir);
    FSPath(const FileSystem &fs, const fs::path &path);

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

    // Returns a pointer to the corresponding FSBlock
    FSBlock *ptr() const;

    // Converts the path to a host path
    fs::path getPath() const;


    //
    // Traverses the directory tree
    //

    // Seeks a file or directory
    Block seek(const FSName &name) const;
    FSPath seekDir(const FSName &name) const;
    FSPath seekFile(const FSName &name) const;
    FSPath seek(const fs::path &path) const;
    FSPath seekDir(const fs::path &path) const;
    FSPath seekFile(const fs::path &path) const;

    // Moves up or down in the directory tree
    void cd(FSName name);
    void cd(const fs::path &path);
    void parent();
};

}
