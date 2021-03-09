// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include <exception>

struct Exception : public std::exception {
    
    string description;
    i64 data;
    
    Exception(const string &s, i64 d) : description(s), data(d) { }
    Exception(const string &s) : description(s), data(0) { }
    Exception(i64 d) : description(""), data(d) { }

    const char *what() const throw() override { return description.c_str(); }
};
