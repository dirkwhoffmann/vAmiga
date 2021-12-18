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
#include <vector>

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

}
