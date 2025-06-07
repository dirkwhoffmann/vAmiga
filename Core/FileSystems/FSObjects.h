// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CoreObject.h"

namespace vamiga {

struct FSString {
    
    // File system identifier
    string str;
    
    // Maximum number of permitted characters
    isize limit = 0;

    static char capital(char c);

    FSString(const string &cppString, isize limit);
    FSString(const char *cString, isize limit);
    FSString(const u8 *bcplString, isize limit);

    const char *c_str() { return str.c_str(); }
    string cpp_str() { return str; }
    
    bool operator== (const FSString &rhs) const;
    isize length() const { return (isize)str.length(); }
    u32 hashValue() const;
    
    void write(u8 *p);
};

struct FSName : FSString {
    
    FSName(const string &cppString);
    FSName(const char *cString);
    FSName(const u8 *bcplString);
    FSName(const fs::path &path);

    fs::path path() const;
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

}
