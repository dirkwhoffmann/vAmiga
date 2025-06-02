// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "FSObjects.h"
#include "FSBlock.h"
#include "Chrono.h"
#include "Macros.h"
#include <algorithm>
#include <cstring>

namespace vamiga {

FSString::FSString(const string &cpp, isize limit) : str(cpp), limit(limit)
{
    
}

FSString::FSString(const char *c, isize l) : limit(l)
{
    assert(c != nullptr);
    
    str.assign(c, strnlen(c, l));
}

FSString::FSString(const u8 *bcpl, isize l) : limit(l)
{
    assert(bcpl != nullptr);
    
    auto length = (isize)bcpl[0];
    auto firstChar = (const char *)(bcpl + 1);
    
    str.assign(firstChar, std::min(length, l));
}

char
FSString::capital(char c)
{
    return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

bool
FSString::operator== (FSString &rhs) const
{
    return util::uppercased(str) == util::uppercased(rhs.str);
}

u32
FSString::hashValue() const
{
    u32 result = (u32)length();
    for (auto c : str) {
        
        result = (result * 13 + (u32)capital(c)) & 0x7FF;
    }

    return result;
}

void
FSString::write(u8 *p)
{
    assert(p != nullptr);
        
    // Write name as a BCPL string (first byte is string length)
    *p++ = (u8)length();
    for (auto c : str) { *p++ = c; }
}

void
FSName::rectify()
{
    // Replace all symbols that are not permitted in Amiga filenames
    /*
    for (isize i = 0; i < isizeof(str); i++) {
        if (str[i] == ':' || str[i] == '/') str[i] = '_';
    }
    */
}

FSTime::FSTime(time_t t)
{
    const u32 secPerDay = 24 * 60 * 60;
    
    // Shift reference point from Jan 1, 1970 (Unix) to Jan 1, 1978 (Amiga)
    t -= (8 * 365 + 2) * secPerDay - 60 * 60;
    
    days = (u32)(t / secPerDay);
    mins = (u32)((t % secPerDay) / 60);
    ticks = (u32)((t % secPerDay % 60) * 50);
}

FSTime::FSTime(const u8 *p)
{
    assert(p != nullptr);
    
    days = FSBlock::read32(p);
    mins = FSBlock::read32(p + 4);
    ticks = FSBlock::read32(p + 8);
}

time_t
FSTime::time() const
{
    const u32 secPerDay = 24 * 60 * 60;
    time_t t = days * secPerDay + mins * 60 + ticks / 50;
    
    // Shift reference point from  Jan 1, 1978 (Amiga) to Jan 1, 1970 (Unix)
    t += (8 * 365 + 2) * secPerDay - 60 * 60;
    
    return t;
}

void
FSTime::write(u8 *p)
{
    assert(p != nullptr);
    
    FSBlock::write32(p + 0, days);
    FSBlock::write32(p + 4, mins);
    FSBlock::write32(p + 8, ticks);
}

string
FSTime::dateStr() const
{
    char tmp[32];
    
    time_t t = time();
    tm local = util::Time::local(t);

    snprintf(tmp, sizeof(tmp), "%04d-%02d-%02d",
             1900 + local.tm_year, 1 + local.tm_mon, local.tm_mday);
    
    return string(tmp);
}

string
FSTime::timeStr() const
{
    char tmp[32];
    
    time_t t = time();
    tm local = util::Time::local(t);

    snprintf(tmp, sizeof(tmp), "%02d:%02d:%02d",
             local.tm_hour, local.tm_min, local.tm_sec);
    
    return string(tmp);
}

string
FSTime::str() const
{
    string result = dateStr() + "  " + timeStr();
    return result;
}

}
