// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"
#include "IOUtils.h"

class Properties : public AmigaObject {

    mutable util::ReentrantMutex mutex;
    
    // Key-value storage
    std::map <string, string> values;

    // Default values (used if no value is set)
    std::map <string, string> defaults;

    
    //
    // Initializing
    //
    
public:
    
    Properties();
    // Properties(Properties const&) = delete;
    // void operator=(Properties const&) = delete;
    
    
    //
    // Methods from AmigaObject
    //
    
public:

    // void prefix() const override;

private:
    
    const char *getDescription() const override { return "Properties"; }
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
    // Working with key-value pairs
    //

public:
    
    string getValue(const string &key) throws;
    i64 getValue(Option option) throws;
    i64 getValue(Option option, isize nr) throws;
    
    string getDefaultValue(const string &key) throws;
    i64 getDefaultValue(Option option) throws;
    i64 getDefaultValue(Option option, isize nr) throws;

    void setValue(const string &key, const string &value);
    void setValue(Option option, i64 value);
    void setValue(Option option, isize nr, i64 value);
    void setValue(Option option, std::vector <isize> nrs, i64 value);

    void setDefaultValue(const string &key, const string &value);
    void setDefaultValue(Option option, i64 value);
    void setDefaultValue(Option option, isize nr, i64 value);
    void setDefaultValue(Option option, std::vector <isize> nrs, i64 value);

    void remove();
    void removeValue(const string &key) throws;
    void removeValue(Option option) throws;
    void removeValue(Option option, isize nr) throws;
};
