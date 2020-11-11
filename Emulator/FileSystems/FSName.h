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

struct FSString {
    
    // File system identifier stored as a C string
    char cStr[92];
    
    // Maximum number of permitted characters
    size_t limit;
    

    FSString(const char *cString, size_t limit);
    FSString(const u8 *bcplString, size_t limit);

    static char capital(char c);
    
    bool operator== (FSString &rhs);
    u32 hashValue();
    
    void write(u8 *p);
};

struct FSName : FSString {
    
    FSName(const char *cString) : FSString(cString, 30) {rectify(); }
    FSName(const u8 *bcplString) : FSString(bcplString, 30) { rectify(); }

    // Scans the given name and replaces invalid characters by dummy symbols
    void rectify();
};

struct FSComment : FSString {
    
    FSComment(const char *cString) : FSString(cString, 91) { }
    FSComment(const u8 *bcplString) : FSString(bcplString, 91) { }
};

#endif
