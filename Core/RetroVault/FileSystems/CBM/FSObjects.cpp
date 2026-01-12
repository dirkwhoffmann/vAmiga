// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystems/PosixViewTypes.h"
#include "FileSystems/CBM/FSObjects.h"
#include "FileSystems/CBM/FSBlock.h"
#include "utl/chrono.h"
#include "utl/support.h"
#include <algorithm>
#include <cstring>
#include <unordered_set>
#include <sys/stat.h>

namespace retro::vault::cbm {

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
FSString::capital(char c, FSFormat dos)
{
    return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

bool
FSString::operator== (const FSString &rhs) const
{
    return utl::uppercased(str) == utl::uppercased(rhs.str);
}

/*
 u32
 FSString::hashValue() const
 {
 u32 result = (u32)length();
 for (auto c : str) {

 result = (result * 13 + (u32)capital(c)) & 0x7FF;
 }

 return result;
 }
 */

u32
FSString::hashValue(FSFormat dos) const
{
    u32 result = (u32)length();
    for (auto c : str) {

        result = (result * 13 + (u32)capital(c, dos)) & 0x7FF;
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
    return utl::uppercased(cpp_str()) < utl::uppercased(other.cpp_str());
}

std::ostream &operator<<(std::ostream &os, const FSString &str) {

    os << str.cpp_str();
    return os;
}

FSName::FSName(const string &cpp) : FSString(cpp, 30) { }
FSName::FSName(const char *c) : FSString(c, 30) { }
FSName::FSName(const u8 *bcpl) : FSString(bcpl, 30) { }
FSName::FSName(const fs::path &path) : FSString(unsanitize(path), 30) { }

fs::path
FSName::sanitize(const string &filename)
{
    auto toUtf8 = [&](u8 c) {

        if (c < 0x80) {
            return string(1, c);
        } else {
            return string(1, char( 0xC0 | (c >> 6))) + char(0x80 | (c & 0x3F));
        }
    };

    auto toHex = [&](u8 c) {

        std::ostringstream ss;
        ss << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        return ss.str();
    };

    auto shouldEscape = [&](u8 c, isize i) {

        // Unhide hidden files
        if (c == '.' && i == 0) return true;

        // Escape the lower ASCII range
        if (c < 23) return true;

        // Escape special characters
        if (c == '<' || c == '>' || c == ':' || c == '"' || c == '\\') return true;
        if (c == '/' || c == '>' || c == '?' || c == '*') return true;

        // Don't escape everything else
        return false;
    };

    auto isReserved = [&](const string& name) {

        static const std::unordered_set<std::string> reserved {
            "CON", "PRN", "AUX", "NUL",
            "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
            "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
        };

        return reserved.count(utl::uppercased(name)) > 0;
    };

    string result;

    // Convert characters one by one
    for (usize i = 0; i < filename.length(); i++) {

        auto u = u8(filename[i]);

        if (u > 127) {
            result += toUtf8(u);
        } else if (shouldEscape(u, i)) {
            result += toHex(u);
        } else {
            result += char(u);
        }
    }

    // Avoid reserved Windows names
    if (isReserved(result)) result = "__" + result;

    /*
     if (filename != result) {
     printf("sanitize: %s -> %s\n", filename.c_str(), result.c_str());
     }
     */

    return fs::path(result);
}

string
FSName::unsanitize(const fs::path &filename)
{
    const auto &s = filename.string();
    const auto len = isize(s.length());

    auto isUtf8 = [&](isize i) {

        if (i + 2 >= len) return false;
        return u8(s[i]) >= 0xC0 && (u8(s[i + 1]) & 0xC0) == 0x80;
    };

    auto fromUtf8 = [&](isize i) {

        return (u8)((((u8)s[i] & 0x1F) << 6) | ((u8)s[i + 1] & 0x3F));
    };

    auto isHexDigit = [&](char c) {

        return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
    };

    auto isHex = [&](isize i) {

        if (i + 3 >= len) return false;
        return s[i] == '%' && isHexDigit(s[i + 1]) && isHexDigit(s[i + 1]);
    };

    auto fromHex = [&](isize i) {

        return std::stoi(s.substr(i + 1, 2), nullptr, 16);
    };

    auto isReserved = [&]() {

        static const std::unordered_set<std::string> reserved {
            "CON", "PRN", "AUX", "NUL",
            "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
            "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
        };

        if (s.rfind("__", 0) != 0) return false;
        return reserved.count(utl::uppercased(s.substr(2))) > 0;
    };

    std::string result;

    if (isReserved()) {

        // Restore the original reserved word
        result = s.substr(2);

    } else {

        // Convert characters one by one
        for (isize i = 0; i < len; i++) {

            if (isUtf8(i)) {
                result += (char)fromUtf8(i); i += 1;
            } else if (isHex(i)) {
                result += (char)fromHex(i); i += 2;
            } else {
                result += s[i];
            }
        }
    }

    /*
     if (filename.string() != result) {
     printf("unsanitize: %s -> %s\n", filename.string().c_str(), result.c_str());
     }
     */

    return result;
}

FSPath::FSPath(const string &s)
{
    // Extract the volume identifier (if any)
    auto vec = split(s, ':');

    switch (vec.size()) {

        case 1: // Relative path

            volume = {};
            for (auto &it : split(vec[0], '/')) {
                if (!it.empty()) components.push_back(FSName(it));
            }
            break;

        case 2: // Absolute path

            volume = FSName(vec[0]);
            for (auto &it : split(vec[1], '/')) {
                if (!it.empty()) components.push_back(FSName(it));
            }
            break;

        default:

            throw FSError(FSError::FS_INVALID_PATH, s);
    }
}

FSPath::FSPath(const fs::path &path)
{
    for (const auto &p : path) {

        if (p == path.root_path()) {

            volume = FSName("");
            continue;
        }
        components.emplace_back(FSName(p));
    }
}

string
FSPath::cpp_str() const
{
    string result;

    for (const auto &p : components) {
        result += (result.empty() ? "" : "/") + p.cpp_str();
    }
    return (absolute() ? ":" : "") + result;
}

FSName
FSPath::filename() const
{
    return components.empty() ? FSName("") : components.back();
}

FSPath
FSPath::parentPath() const
{
    FSPath result = *this;

    if (!result.components.empty()) {
        result.components.pop_back();
    }

    return result;
}

FSPath &
FSPath::operator/=(const FSName &name)
{
    components.push_back(name);
    return *this;
}

FSPath &
FSPath::operator/=(const FSPath &other)
{
    // If other is absolute, replace the entire path
    if (other.absolute()) { *this = other; return *this; }

    components.insert(components.end(),
                      other.components.begin(),
                      other.components.end());

    return *this;
}

FSPath
FSPath::operator/(const FSName &rhs) const
{
    FSPath copy = *this;
    copy /= rhs;
    return copy;
}

FSPath
FSPath::operator/(const FSPath &rhs) const
{
    FSPath copy = *this;
    copy /= rhs;
    return copy;
}

FSPattern::FSPattern(const string glob) : glob(glob)
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
        throw FSError(FSError::FS_INVALID_REGEX, glob);
    }
}

std::vector<FSPattern>
FSPattern::splitted() const
{
    std::vector<FSPattern> result;

    // Extract the volume identifier (if any)
    auto vec = split(glob, ':');

    switch (vec.size()) {

        case 1: // Relative path

            for (auto &it : split(vec[0], '/')) {
                if (!it.empty()) result.push_back(FSPattern(it));
            }
            break;

        case 2: // Absolute path

            result.push_back(FSPattern(vec[0] + ":"));
            for (auto &it : split(vec[1], '/')) {
                if (!it.empty()) result.push_back(FSPattern(it));
            }
            break;

        default:

            throw FSError(FSError::FS_INVALID_PATH, glob);
    }

    return result;
}

bool
FSPattern::match(const FSString &name) const
{
    return std::regex_match(name.cpp_str(), regex);
}

}
