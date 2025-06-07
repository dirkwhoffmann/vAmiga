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

namespace vamiga {

struct FSPath
{
    const class FileSystem &fs;

    Block dir;

    FSPath(const FSPath &);
    FSPath(const FileSystem &fs, Block dir);
    FSPath(const FileSystem &fs, class FSBlock *dir);

    FSPath &operator=(const FSPath &);

    void selfcheck();

    class FSBlock *ptr();

    bool isRoot();
    bool isFile();
    bool isDirectory();

    Block seekRef(FSName name) const;

    //
    // Working with directories
    //

    // Moves one level down in the directory tree
    void cd(FSName name);
    FSPath& operator/=(const FSName &name);
    FSPath operator/(const FSName &name) const;

    // Moves one level up in the directory tree
    void parent();

    fs::path getPath() const;
};

}
