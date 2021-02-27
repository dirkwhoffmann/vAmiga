// -----------------------------------------------------------------------------
// This file is part of vAmiga Bare Metal
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include <exception>

struct Exception : public std::exception {
    
    string description;
    i64 data = 0;
    
    Exception(const string &s) : description(s) { }
    Exception(const string &s, i64 d) : description(s), data(d) { }
    
    const char *what() const throw() override { return description.c_str(); }
};
