// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_OBJECTS_H
#define _FS_OBJECTS_H

#include "AmigaObject.h"

struct FSString : AmigaObject {
    
    // File system identifier stored as a C string
    char str[92];
    
    // Maximum number of permitted characters
    size_t limit;

    static char capital(char c);

    FSString(const char *cString, size_t limit);
    FSString(const u8 *bcplString, size_t limit);

    const char *c_str() { return str; }

    bool operator== (FSString &rhs) const;
    u32 hashValue() const;
    
    void write(u8 *p);
};

struct FSName : FSString {
    
    FSName(const char *cString) : FSString(cString, 30) {rectify(); }
    FSName(const u8 *bcplString) : FSString(bcplString, 30) { rectify(); }

    const char *getDescription() const override { return "FSName"; }

    // Scans the given name and replaces invalid characters by dummy symbols
    void rectify();
};

struct FSComment : FSString {
    
    FSComment(const char *cString) : FSString(cString, 91) { }
    FSComment(const u8 *bcplString) : FSString(bcplString, 91) { }

    const char *getDescription() const override { return "FSComment"; }
};

struct FSTime : AmigaObject {
    
    u32 days;
    u32 mins;
    u32 ticks;
    
    FSTime(time_t t);
    FSTime(const u8 *p);

    const char *getDescription() const override { return "FSTime"; }

    time_t time() const;
    void write(u8 *p);

    string dateStr() const;
    string timeStr() const;
    string str() const;
};

#endif
