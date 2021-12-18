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

namespace util {

//
// Parsing strings
//

bool parseHex(const string &s, isize *result);


//
// Changing capitalization
//

string lowercased(const string& s);
string uppercased(const string& s);


//
// Splitting string
//

std::vector<string> split(const string &s, char delimiter);

}
