// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/common.h"

namespace retro::vault {

using namespace utl;

struct FSPosixAttr {

    // Meta data

    isize size;         // File size in bytes
    isize blocks;       // Number of occupied blocks
    u32 prot;           // Protection flags
    bool isDir;         // Is it a directory?

    // Access times

    time_t btime;       // Time of birth
    time_t atime;       // Time of last access
    time_t mtime;       // Time of last data modification
    time_t ctime;       // Time of last status change
};

struct FSPosixStat {

    // Meta data

    string name;
    isize bsize;        // Block size
    isize blocks;       // File system capacity in blocks

    // Usage information

    isize freeBlocks;   // Available blocks
    isize usedBlocks;   // Occupied blocks

    // Access times

    time_t btime;       // Time of birth
    time_t mtime;       // Time of last data modification

    // Access statistics

    isize blockReads;   // Total number of read blocks
    isize blockWrites;  // Total number of written blocks
};

struct Handle {

    isize id;           // Unique indentifier
    isize node;         // File root node
    isize offset;       // I/O offset
    u32 flags;          // Open mode
};

using HandleRef = isize;

}
