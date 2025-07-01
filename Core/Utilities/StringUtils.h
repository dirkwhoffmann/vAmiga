// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BasicTypes.h"

namespace vamiga::util {

//
// Creating
//

// Creates a string from a buffer
string createStr(const u8 *buf, isize maxLen);
string createAscii(const u8 *buf, isize len, char fill = '.');

// Creates a string by repeating a substring multiple times
string repeat(const string &s, isize times);


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
// Stripping off characters and 
//

string ltrim(const string &s, const string &characters = " ");
string rtrim(const string &s, const string &characters = " ");
string trim(const string &s, const string &characters = " ");

// Returns the common prefix of multiple strings
string commonPrefix(const string &s1, const string &s2, bool caseSensitive = true);
string commonPrefix(const std::vector<string> &vec, bool caseSensitive = true);


//
// Splitting and concatenating
//

std::vector<string> split(const string &s, char delim);
std::vector<string> split(const std::vector<string> &vec, char delim);

std::pair<vector<string>, string> splitLast(const vector<string> &vec);

string concat(const std::vector<string> &s,
              const string &delim = " ", const string &ldelim = "", const string &rdelim = "");


//
// Pretty printing
//

// Returns a textual description for a byte count
string byteCountAsString(isize bytes);

// Returns a textual description for a fill level
string fillLevelAsString(double percentage);

}
