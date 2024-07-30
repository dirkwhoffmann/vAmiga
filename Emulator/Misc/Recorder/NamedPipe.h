// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BasicTypes.h"

namespace vamiga {

class NamedPipe {
    
    int pipe = -1;
    string name;
    
public:
    
    bool create(const string &name);
    bool open();
    bool isOpen();
    bool close();
    isize write(u8 *buffer, isize length);
};

}
