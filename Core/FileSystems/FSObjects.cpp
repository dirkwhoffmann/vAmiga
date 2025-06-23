// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSObjects.h"
#include "FSBlock.h"
#include "Chrono.h"
#include "Macros.h"
#include "Host.h"

#include <algorithm>
#include <cstring>
#include <unordered_set>

namespace vamiga {

FSString::FSString(const string &cpp, isize limit) : str(cpp), limit(limit)
{
    
}

FSString::FSString(const char *c, isize limit) : limit(limit)
{
    assert(c != nullptr);
    
    str.assign(c, strnlen(c, limit));
}

FSString::FSString(const u8 *bcpl, isize limit) : limit(limit)
{
    assert(bcpl != nullptr);
    
    auto length = (isize)bcpl[0];
    auto firstChar = (const char *)(bcpl + 1);
    
    str.assign(firstChar, std::min(length, limit));
}

char
FSString::capital(char c)
{
    return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

bool
FSString::operator== (const FSString &rhs) const
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

bool
FSString::operator<(const FSString& other) const
{
    return util::uppercased(cpp_str()) < util::uppercased(other.cpp_str());
}

std::ostream &operator<<(std::ostream &os, const FSString &str) {

    os << str.cpp_str();
    return os;
}

FSName::FSName(const string &cpp) : FSString(cpp, 30) { }
FSName::FSName(const char *c) : FSString(c, 30) { }
FSName::FSName(const u8 *bcpl) : FSString(bcpl, 30) { }
FSName::FSName(const fs::path &path) : FSString(Host::unsanitize(path), 30) { }

fs::path
FSName::path() const
{
    return Host::sanitize(str);
}

FSPattern::FSPattern(const string glob)
{
    // Create regex string
    std::string re = "^";

    for (char c : glob) {

        switch (c) {

            case '*': re += ".*"; break;
            case '?': re += "."; break;
            case '.': re += "\\."; break;
            case '\\': re += "\\\\"; break;

            default:
                if (std::isalnum(u8(c))) {
                    re += c;
                } else {
                    re += '\\';
                    re += c;
                }
        }
    }
    re += "$";

    try {
        regex = std::regex(re, std::regex::ECMAScript | std::regex::icase);
    } catch (const std::regex_error &) {
        throw AppError(Fault::FS_INVALID_REGEX, glob);
    }
}

bool
FSPattern::match(const FSName &name)
{
    return std::regex_match(name.cpp_str(), regex);
}

FSTime::FSTime(time_t t)
{
    const u32 secPerDay = 24 * 60 * 60;
    
    // Shift reference point from Jan 1, 1970 (Unix) to Jan 1, 1978 (Amiga)
    t -= (8 * 365 + 2) * secPerDay;

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
    
    // Shift reference point from Jan 1, 1978 (Amiga) to Jan 1, 1970 (Unix)
    t += (8 * 365 + 2) * secPerDay;

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
    const char *month[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    char tmp[32];
    
    time_t t = time();
    tm gm = util::Time::gmtime(t);
    snprintf(tmp, sizeof(tmp), "%02d-%s-%02d", gm.tm_mday, month[gm.tm_mon % 12], gm.tm_year);

    return string(tmp);
}

string
FSTime::timeStr() const
{
    char tmp[32];
    
    time_t t = time();
    tm local = util::Time::gmtime(t);

    snprintf(tmp, sizeof(tmp), "%02d:%02d:%02d",
             local.tm_hour, local.tm_min, local.tm_sec);
    
    return string(tmp);
}

string
FSTime::str() const
{
    string result = dateStr() + " " + timeStr();
    return result;
}

}
