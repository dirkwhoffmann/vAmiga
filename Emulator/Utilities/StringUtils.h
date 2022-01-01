// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include <vector>

namespace util {

//
// Converting
//

// Parses a hexadecimal number in string format
bool parseHex(const string &s, isize *result);

// Converts an integer value to a hexadecimal string representation
template <isize digits> string hexstr(isize number);


//
// Transforming
//

// Converts the capitalization of a string
string lowercased(const string& s);
string uppercased(const string& s);

// Replaces all unprintable characters
string makePrintable(const string& s);


//
// Stripping off characters
//

string ltrim(const string &s, const string &characters);
string rtrim(const string &s, const string &characters);


//
// Splitting and concatenating
//

std::vector<string> split(const string &s, char delimiter);
string concat(std::vector<string> &s, string delimiter);

}
