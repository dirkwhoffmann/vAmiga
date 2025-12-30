// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/abilities.h"
#include "utl/storage.h"
#include <sstream>
#include <fstream>

namespace vamiga {

using namespace utl;

class AnyFile : public Hashable, public Dumpable, public Loggable {

public:
    
    // The location of this file (may be empty)
    fs::path path;

    // The raw data of this file
    Buffer<u8> data;
    
    
    //
    // Initializing
    //
    
public:

    virtual ~AnyFile() = default;

    void init(isize len);
    void init(const u8 *buf, isize len);
    void init(const Buffer<u8> &buffer);
    void init(const string &str);
    void init(const fs::path &path);


    //
    // Methods from Hashable
    //

public:

    u64 hash(HashAlgorithm algorithm) const override {
        return data.hash(algorithm);
    }


    //
    // Methods from Dumpable
    //

public:

    Dumpable::DataProvider dataProvider() const override {
        return data.dataProvider();
    }


    //
    // Accessing
    //
    
public:

    isize getSize() const { return data.size; }
    u8* getData() const { return data.ptr; }
    bool empty() const { return data.empty(); }

    // Copies the file contents into a buffer
    virtual void copy(u8 *dst, isize offset, isize len) const;
    virtual void copy(u8 *dst, isize offset = 0) const;


    //
    // Serializing
    //
    
public:

    virtual bool isCompatiblePath(const fs::path &path) const = 0;

    isize writeToStream(std::ostream &stream) const;
    isize writeToFile(const fs::path &path) const;
    // isize writeToBuffer(u8 *buf) const;
    // isize writeToBuffer(Buffer<u8> &buffer) const;

    isize writeToStream(std::ostream &stream, isize offset, isize len) const;
    isize writeToFile(const fs::path &path, isize offset, isize len) const;
    // isize writeToBuffer(u8 *buf, isize offset, isize len) const;
    // isize writeToBuffer(Buffer<u8> &buffer, isize offset, isize len) const;

private:
    
    // Delegation methods
    virtual void finalizeRead() { };
};

}
