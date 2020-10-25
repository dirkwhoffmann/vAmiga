// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSName.h"
#include "Utils.h"

FSName::FSName(const char *str)
{
    assert(str != nullptr);
    
    // Amiga file and volume names are limited to 30 characters
    strncpy(name, str, 30);
    
    // Replace all symbols that are not permitted in Amiga filenames
    for (size_t i = 0; i < sizeof(name); i++) {
        if (name[i] == ':' || name[i] == '/') name[i] = '_';
    }
    
    // Make sure the string terminates
    name[30] = 0;
}

u32
FSName::hash()
{
    size_t length = strlen(name);
    u32 result = (u32)length;
    
    for (size_t i = 0; i < length; i++) {
        char c = name[i];
        if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
        result = (result * 13 + (u32)c) & 0x7FF;
    }
    return result % 72;
}

void
FSName::write(u8 *p)
{
    assert(p != nullptr);
    assert(strlen(name) <= 30);
    
    // Write name as BCPL string (first byte is string length)
    p[0] = strlen(name);
    strncpy((char *)(p + 1), name, strlen(name));
}
