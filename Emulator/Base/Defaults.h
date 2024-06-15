// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CoreComponent.h"
#include "IOUtils.h"

namespace vamiga {

class Defaults : public CoreObject {

    mutable util::ReentrantMutex mutex;
    
    // Key-value storage
    std::map <string, string> values;

    // Fallback values (used if no value is set)
    std::map <string, string> fallbacks;

    
    //
    // Initializing
    //
    
public:
    
    Defaults();
    Defaults(Defaults const&) = delete;
    void operator=(Defaults const&) = delete;
    
    
    //
    // Methods from CoreObject
    //

private:
    
    const char *objectName() const override { return "Defaults"; }
    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Loading and saving
    //
    
public:
    
    // Loads a properties file from disk
    void load(const fs::path &path) throws;
    void load(std::ifstream &stream) throws;
    void load(std::stringstream &stream) throws;

    // Saves a properties file to disk
    void save(const fs::path &path) throws;
    void save(std::ofstream &stream) throws;
    void save(std::stringstream &stream) throws;

    
    //
    // Reading key-value pairs
    //

public:
    
    // Queries a key-value pair
    string getString(const string &key) const throws;
    i64 getInt(const string &key) const throws;
    i64 get(Option option) const throws;
    i64 get(Option option, isize nr) const throws;
    
    // Queries a fallback key-value pair
    string getFallbackString(const string &key) const;
    i64 getFallbackInt(const string &key) const;
    i64 getFallback(Option option) const;
    i64 getFallback(Option option, isize nr) const;


    //
    // Writing key-value pairs
    //

    void setString(const string &key, const string &value);
    void set(Option option, i64 value);
    void set(Option option, isize nr, i64 value);
    void set(Option option, std::vector <isize> nrs, i64 value);

    void setFallback(const string &key, const string &value);
    void setFallback(Option option, const string &value);
    void setFallback(Option option, i64 value);
    void setFallback(Option option, isize nr, const string &value);
    void setFallback(Option option, isize nr, i64 value);
    void setFallback(Option option, std::vector <isize> nrs, const string &value);
    void setFallback(Option option, std::vector <isize> nrs, i64 value);

    void remove();
    void remove(const string &key) throws;
    void remove(Option option) throws;
    void remove(Option option, isize nr) throws;
    void remove(Option option, std::vector <isize> nrs) throws;
};

}
