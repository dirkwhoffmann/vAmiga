// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaObject.h"

struct FSString {
    
    // File system identifier stored as a C string
    char str[92];
    
    // Maximum number of permitted characters
    isize limit;

    static char capital(char c);

    FSString(const string &cppString, isize limit);
    FSString(const char *cString, isize limit);
    FSString(const u8 *bcplString, isize limit);

    const char *c_str() { return str; }
    string cpp_str() { return string(str); }
    
    bool operator== (FSString &rhs) const;
    u32 hashValue() const;
    
    void write(u8 *p);
};

struct FSName : FSString {
    
    FSName(const string &cppString) : FSString(cppString, 30) { rectify(); }
    FSName(const char *cString) : FSString(cString, 30) { rectify(); }
    FSName(const u8 *bcplString) : FSString(bcplString, 30) { rectify(); }
    
    // Scans the given name and replaces invalid characters by dummy symbols
    void rectify();
};

struct FSComment : FSString {
    
    FSComment(const char *cString) : FSString(cString, 91) { }
    FSComment(const u8 *bcplString) : FSString(bcplString, 91) { }
};

struct FSTime {
    
    u32 days;
    u32 mins;
    u32 ticks;
    
    FSTime(time_t t);
    FSTime(const u8 *p);

    time_t time() const;
    void write(u8 *p);

    string dateStr() const;
    string timeStr() const;
    string str() const;
};
