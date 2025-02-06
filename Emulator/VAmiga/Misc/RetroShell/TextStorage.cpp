// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "TextStorage.h"
#include "Amiga.h"

namespace vamiga {

string
TextStorage::operator [] (isize i) const
{
    assert(i >= 0 && i < size());
    return storage[i];
}

string&
TextStorage::operator [] (isize i)
{
    assert(i >= 0 && i < size());
    return storage[i];
}

void
TextStorage::text(string &all)
{
    auto count = size();
    
    all = "";
    for (isize i = 0; i < count; i++) {
        
        all += storage[i];
        if (i < count - 1) all += '\n';
    }
}

void
TextStorage::clear()
{
    storage.clear();
    storage.push_back("");
}

bool
TextStorage::isCleared()
{
    return storage.size() == 1 && storage[0].size() == 0;
}

bool 
TextStorage::lastLineIsEmpty()
{
    return storage.back().empty();
}

void
TextStorage::append(const string &line)
{
    storage.push_back(line);

    // Remove old entries if the storage grows too large
    while (storage.size() > capacity) storage.erase(storage.begin());
}

TextStorage&
TextStorage::operator<<(char c)
{
    assert(!storage.empty());

    switch (c) {
            
        case '\n':
            
            if (ostream) *ostream << storage.back() << std::endl;

            append("");
            break;
            
        case '\r':

            storage.back() = "";
            break;
            
        default:
            
            if (isprint(c)) storage.back() += c;
            break;
    }
    
    return *this;
}

TextStorage&
TextStorage::operator<<(const string &s)
{
    for (auto &c : s) *this << c;
    return *this;
}

TextStorage &
TextStorage::operator<<(std::stringstream &ss)
{
    string line;
    while(std::getline(ss, line)) *this << line << '\n';
    return *this;
}

}
