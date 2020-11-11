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

struct FSName {
    
    char name[30 + 1];
    
    FSName(const char *str);
    FSName(const u8 *bcplStr);
    
    char capital(char c);
    bool operator== (FSName &rhs);
    u32 hashValue();
  
    void dump() {};
    void write(u8 *ptr);
};

struct FSComment {
    
    char name[91 + 1];
    
    FSComment(const char *str);
    FSComment(const u8 *bcplStr);

    void dump() {};
    void write(u8 *ptr);
};

#endif
