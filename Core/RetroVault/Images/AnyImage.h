// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AnyFile.h" // REMOVE ASAP
#include "ImageTypes.h"
#include "utl/abilities.h"
#include "utl/storage.h"
#include <iostream>

namespace vamiga {

using namespace utl;

// REMOVE ASAP
using AnyImage = AnyFile;

/*

// Base class for all disk images
class AnyImage : public Hashable, public Dumpable, public Loggable {

public:

    // The location of this file (may be empty)
    fs::path path;

    // The raw data of this file
    Buffer<u8> data;


    //
    // Initializing
    //

public:

    virtual ~AnyImage() = default;

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
    // Querying meta information
    //

public:

    isize getSize() const { return data.size; }
    u8* getData() const { return data.ptr; }
    bool empty() const { return data.empty(); }

    // Returns meta-information about the file
    virtual std::vector<string> describe() const { return {}; }


    //
    // Accessing data
    //

public:

    ByteView byteView(isize offset = 0) const;
    ByteView byteView(isize offset, isize len) const;
    MutableByteView byteView(isize offset = 0);
    MutableByteView byteView(isize offset, isize len);

    // Copies the file contents into a buffer
    virtual void copy(u8 *dst, isize offset, isize len) const;
    virtual void copy(u8 *dst, isize offset = 0) const;


    //
    // Importing
    //

    // Returns true if path points to a compatible file
    virtual bool isCompatiblePath(const fs::path &path) const { return true; }


    //
    // Exporting
    //

public:

    isize writeToStream(std::ostream &stream) const;
    isize writeToFile(const fs::path &path) const;

    isize writeToStream(std::ostream &stream, isize offset, isize len) const;
    isize writeToFile(const fs::path &path, isize offset, isize len) const;

private:

    // Called at the end of init()
    virtual void didLoad() {};
};
*/

}
