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
// Changing capitalization
//

string lowercased(const string& s);
string uppercased(const string& s);


//
// Parsing strings
//

bool parseHex(const string &s, isize *result);

}
