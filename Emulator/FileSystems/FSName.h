// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_NAME_H
#define _FS_NAME_H

#include "Aliases.h"

class FSName {
    
    char name[31];
    
public:

    FSName(const char *str);

    u32 hash();
    void write(u8 *ptr);
};

#endif
