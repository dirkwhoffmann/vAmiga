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
#include <ostream>
#include <regex>

namespace vamiga {

struct FSString {
    
    // File system identifier
    string str;
    
    // Maximum number of permitted characters
    isize limit = 0;

    static char capital(char c);

    FSString(const string &cppS, isize limit = 1024);
    FSString(const char *c, isize limit = 1024);
    FSString(const u8 *bcpl, isize limit = 1024);
    FSString(const std::map<string,string> map, const string &cpp, const string alt = "", isize limit = 1024);

    const char *c_str() const { return str.c_str(); }
    string cpp_str() const { return str; }

    bool operator== (const FSString &rhs) const;
    isize length() const { return (isize)str.length(); }
    bool empty() const { return str.empty(); }
    u32 hashValue() const;

    void write(u8 *p);

    bool operator<(const FSString& other) const;
    friend std::ostream &operator<<(std::ostream &os, const FSString &str);
};

struct FSName : FSString {
    
    FSName(const string &cpp);
    FSName(const char *c);
    FSName(const u8 *bcpl);
    FSName(const fs::path &path);
    FSName(const std::map<string,string> map, const string &cpp, const string fallback);

    fs::path path() const;
};

struct FSComment : FSString {
    
    FSComment(const char *cString) : FSString(cString, 91) { }
    FSComment(const u8 *bcplString) : FSString(bcplString, 91) { }
};

struct FSPattern {

    std::regex regex;

    FSPattern(const string str);

    bool match(const FSName &name);
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
