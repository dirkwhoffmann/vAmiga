// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CoreObject.h"
#include "OptionTypes.h"
#include "Synchronizable.h"
#include "IOUtils.h"

namespace vamiga {

class Defaults : public CoreObject, public Synchronizable {

    // The key-value storage
    std::map <string, string> values;

    // The default value storage
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
    string getRaw(const string &key) const throws;
    i64 get(const string &key) const throws;
    i64 get(Option option, isize nr = 0) const throws;

    // Queries a fallback key-value pair
    string getFallbackRaw(const string &key) const;
    i64 getFallback(const string &key) const;
    i64 getFallback(Option option, isize nr = 0) const;


    //
    // Writing key-value pairs
    //

    void set(const string &key, const string &value);
    void set(Option option, const string &value, std::vector<isize> objids = { 0 });
    void set(Option option, i64 value, std::vector<isize> objids = { 0 });

    void setFallback(const string &key, const string &value);
    void setFallback(Option option, const string &value, std::vector<isize> objids = { 0 });
    void setFallback(Option option, i64 value, std::vector<isize> objids = { 0 });

    void remove();
    void remove(const string &key) throws;
    void remove(Option option, std::vector<isize> objids = { 0 }) throws;
};

}
