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

    // Key-value storage
    std::map <string, string> values;

    // Default values (used if no value is set)
    std::map <string, string> defaults;

    
    //
    // Initializing
    //
    
public:
    
    Properties();
    Properties(Properties const&) = delete;
    void operator=(Properties const&) = delete;
    
    
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

    // Reads a value
    string getValue(const string &key) throws;
    i64 getValue(Option key) throws;
    i64 getValue(Option key, isize nr) throws;
    string getDefaultValue(const string &key) throws;
    i64 getDefaultValue(Option key) throws;
    i64 getDefaultValue(Option key, isize nr) throws;

    // Writes a value
    void setValue(const string &key, const string &value);
    void setValue(Option key, i64 value);
    void setValue(Option key, isize nr, i64 value);
    void setDefaultValue(const string &key, const string &value);
    void setDefaultValue(Option key, i64 value);
    void setDefaultValue(Option key, isize nr, i64 value);
    void setDefaultValue(Option key, std::vector <isize> nrs, i64 value);

    // Removes a value
    void removeValue(const string &key) throws;
    void removeValue(Option key) throws;
    void removeValue(Option key, isize nr) throws;
    void removeAll();
};
