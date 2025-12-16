// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

/*
#include "utl/storage.h"

#pragma once

namespace vamiga {

using namespace utl;

class BlockStorage {

    friend class Device;
    
    // Number of blocks in this storage
    isize capacity = 0;

    // Block size in bytes
    isize bsize = 0;

    // Data storage
    std::vector<std::unique_ptr<Buffer<u8>>> blocks;

public:

    void init(isize capacity, isize bsize = 512);

    void freeBlock(isize nr);
    Buffer<u8> *readBlock(isize nr);
    Buffer<u8> *ensureBlock(isize nr);
};

}
*/
