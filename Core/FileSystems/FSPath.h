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

    FSPath(const FileSystem &fs, Block dir);

    void selfcheck();

    bool isRoot();
    bool isFile();
    bool isDirectory();

    Block seekRef(FSName name) const;

    void cd(FSName name);
    void parent();

    FSPath& operator/= (const FSName &name);
    FSPath operator/ (const FSName &name) const;

    fs::path getPath() const;
};

}
