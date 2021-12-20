// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "StringUtils.h"
#include <sstream>

namespace util {

bool
parseHex(const string &s, isize *result)
{
    try {
        size_t pos = 0;
        auto num = std::stol(s, &pos, 16);

        if (pos == s.size()) {

            *result = (isize)num;
            return true;
        }
        return false;
    } catch (...) {
        return false;
    }
}

string
lowercased(const string& s)
{
    string result;
    for (auto c : s) { result += (char)std::tolower(c); }
    return result;
}

string
uppercased(const string& s)
{
    string result;
    for (auto c : s) { result += (char)std::toupper(c); }
    return result;
}

string
makePrintable(const string& s)
{
    string result;
    for (auto c : s) {
        if (isprint(c)) {
            result += c;
        } else {
            result += "[" + hexstr<2>(c) + "]";
        }
    }
    return result;
}

std::vector<string>
split(const string &s, char delimiter)
{
    std::stringstream ss(s);
    std::vector<std::string> result;
    string substr;
    
    while(std::getline(ss, substr, delimiter)) {
        result.push_back(substr);
    }
    
    return result;
}

template <isize digits> string hexstr(isize number)
{
    char str[digits + 1];
    
    str[digits] = 0;
    for (isize i = digits - 1; i >= 0; i--, number >>= 4) {
        
        auto nibble = number & 0xF;
        str[i] = (char)(nibble < 10 ? '0' + nibble : 'a' + (nibble - 10));
    }
    
    return string(str, digits);
}

template string hexstr <1> (isize number);
template string hexstr <2> (isize number);
template string hexstr <4> (isize number);
template string hexstr <6> (isize number);
template string hexstr <8> (isize number);
template string hexstr <16> (isize number);
template string hexstr <32> (isize number);
template string hexstr <64> (isize number);

}
