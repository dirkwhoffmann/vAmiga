// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "TextStorage.h"

const char *
TextStorage::text()
{
    static string all;
    
    all = "";

    if (auto numRows = storage.size()) {
        
        // Add all rows except the last one
        for (usize i = 0; i < numRows - 1; i++) all += storage[i] + "\n";
        
        // Add the last row
        all += storage[numRows - 1] + " ";
    }
    
    return all.c_str();
}

void
TextStorage::clear()
{
    storage.clear();
    storage.push_back("");
}

void
TextStorage::append(const string &line)
{
    storage.push_back(line);
 
    // Remove old entries if the storage grows too large
    while (storage.size() > capacity) storage.erase(storage.begin());
}

void
TextStorage::add(char c)
{
    switch (c) {
            
        case '\n':
            
            append(input);
            input = "";
            return;
            
        case '\r':
            
            input = "";
            return;
            
        default:
            
            input += c;
    }
}

void
TextStorage::add(const string &str)
{
    for (auto c : str) add(c);
}

void
TextStorage::insert(isize pos, char c)
{
    if (pos < (isize)input.size()) {
        input.insert(input.begin() + pos, c);
    } else {
        add(c);
    }
}

void
TextStorage::remove(isize pos)
{
    if (pos < (isize)input.size()) {
        input.erase(input.begin() + pos);
    }
}

void
TextStorage::tab(isize pos)
{
    isize delta = pos - (isize)input.size();
    for (isize i = 0; i < delta; i++) input += " ";
}
