// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Macros.h"
#include <sstream>
#include <fstream>
#include <vector>

class TextStorage {

    // Maximum number if lines
    static constexpr usize capacity = 512;
    
    // Main storage (not editable)
    std::vector<string> storage;
    
    // Input line (editable)
    string input;
    
    // Input prompt
    string prompt = "vAmiga% ";
    
    
    //
    // Working with the text storage
    //

public:

    // Returns the contents of the whole storage as a single C string
    const char *text();
    
    // Returns the number of stored lines
    isize size() { return (isize)storage.size(); }
    
    // Returns a reference to the last line
    [[deprecated]] string &back() { return storage.back(); }
    
    // Initializes the storage with a single empty line
    void clear();
    
    // Appends a new line
    void append(const string &line);
    
    // Operator overloads
    string operator [] (isize i) const { return storage[i]; }
    string& operator [] (isize i) { return storage[i]; }

    
    //
    // Working with the input line
    //
    
public:

    // Getter and setter
    string getInput() { return input; }
    void setInput(const string &str) { input = str; }
    isize inputLength() { return (isize)input.length(); }
    
    // Appends a single character or a string at the end
    void add(char c);
    void add(const string &str);
    
    // Inserts a character at a certain position
    void insert(isize pos, char c);
    
    // Removes a character at a certain position
    void remove(isize pos);
    
    //
    void tab(isize pos);
};
