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
#include <sstream>
#include <fstream>

namespace vamiga {

class TextStorage {

    // Maximum number of stored lines
    static constexpr usize capacity = 512;
    
    // The stored lines
    std::vector<string> storage;

public:
    
    // Optional output stream for debugging
    std::ostream *ostream = nullptr;

    
    //
    // Reading
    //
    
public:
    
    // Returns the number of stored lines
    isize size() const { return (isize)storage.size(); }

    // Returns a single line
    string operator [] (isize i) const;
    string& operator [] (isize i);

    // Returns the whole storage contents
    void text(string &all);

    
    //
    // Writing
    //
    
public:
    
    // Initializes the storage with a single empty line
    void clear();

    // Returns true if the console is cleared
    bool isCleared();

    // Returns true if the last line contains no text
    bool lastLineIsEmpty();

private:
    
    // Appends a new line
    void append(const string &line);

public:
    
    // Appends a single character or a string
    TextStorage &operator<<(char c);
    TextStorage &operator<<(const string &s);
    TextStorage &operator<<(std::stringstream &ss);
};

}
