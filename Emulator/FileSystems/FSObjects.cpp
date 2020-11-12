// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSObjects.h"
#include "Utils.h"

FSString::FSString(const char *cStr, size_t l) : limit(l)
{
    assert(cStr != nullptr);
    assert(limit <= 91);
    
    strncpy(this->cStr, cStr, limit);
    this->cStr[limit] = 0;
}

FSString::FSString(const u8 *bcplStr, size_t l) : limit(l)
{
    assert(bcplStr != nullptr);
    assert(limit <= 91);

    // First entry of BCPL string contains the string length
    u8 len = MIN(bcplStr[0], limit);

    strncpy(cStr, (const char *)(bcplStr + 1), limit);
    cStr[len] = 0;
}

char
FSString::capital(char c)
{
    return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

bool
FSString::operator== (FSString &rhs)
{
    int n = 0;
    
    while (cStr[n] != 0 || rhs.cStr[n] != 0) {
        if (capital(cStr[n]) != capital(rhs.cStr[n])) return false;
        n++;
    }
    return true;
}

u32
FSString::hashValue()
{
    size_t length = strlen(cStr);
    u32 result = (u32)length;
    
    for (size_t i = 0; i < length; i++) {
        char c = capital(cStr[i]);
        result = (result * 13 + (u32)c) & 0x7FF;
    }
    return result;
}

void
FSString::write(u8 *p)
{
    assert(p != nullptr);
    assert(strlen(cStr) < sizeof(cStr));

    // Write name as a BCPL string (first byte is string length)
    p[0] = strlen(cStr);
    strncpy((char *)(p + 1), cStr, strlen(cStr));
}

void
FSName::rectify()
{
    // Replace all symbols that are not permitted in Amiga filenames
    for (size_t i = 0; i < sizeof(cStr); i++) {
        if (cStr[i] == ':' || cStr[i] == '/') cStr[i] = '_';
    }
}


/*

FSName::FSName(const u8 *bcplStr)
{
    assert(bcplStr != nullptr);
    
    // First entry is string length
    u8 len = *(bcplStr++);
    
    // Amiga file and volume names are limited to 30 characters
    if (len <= 30) strncpy(name, (const char *)bcplStr, 30);
    
    // Make sure the string terminates
    name[30] = 0;
}

char
FSName::capital(char c)
{
    return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

bool
FSName::operator== (FSName &rhs)
{
    int n = 0;
    
    while (name[n] != 0 || rhs.name[n] != 0) {
        if (capital(name[n]) != capital(rhs.name[n])) return false;
        n++;
    }
    return true;
}

u32
FSName::hashValue()
{
    size_t length = strlen(name);
    u32 result = (u32)length;
    
    for (size_t i = 0; i < length; i++) {
        char c = capital(name[i]);
        result = (result * 13 + (u32)c) & 0x7FF;
    }
    return result % 72;
}

void
FSName::write(u8 *p)
{
    assert(p != nullptr);
    assert(strlen(name) < sizeof(name));

    // Write name as BCPL string (first byte is string length)
    p[0] = strlen(name);
    strncpy((char *)(p + 1), name, strlen(name));
}

FSComment::FSComment(const char *str)
{
    assert(str != nullptr);
    
    // Comments are limited to 91 characters
    strncpy(name, str, 91);
        
    // Make sure the string terminates
    name[90] = 0;
}

FSComment::FSComment(const u8 *bcplStr)
{
    assert(bcplStr != nullptr);
    
    // First entry is string length
    u8 len = *(bcplStr++);
    
    // Amiga file and volume names are limited to 91 characters
    if (len <= 91) strncpy(name, (const char *)bcplStr, 91);
    
    // Make sure the string terminates
    name[91] = 0;
}

void
FSComment::write(u8 *p)
{
    assert(p != nullptr);
    assert(strlen(name) < sizeof(name));
    
    // Write name as BCPL string (first byte is string length)
    p[0] = strlen(name);
    strncpy((char *)(p + 1), name, strlen(name));
}
*/
