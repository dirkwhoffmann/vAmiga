// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "utl/abilities/Hashable.h"
#include "utl/abilities/Dumpable.h"
#include "utl/abilities/Loggable.h"
#include "utl/storage/Buffer.h"
#include <iosfwd>

namespace vamiga {


// Base class for all file-backed binary formats
/// Base class of all files that can be loaded or saved.
class AnyFile : public utl::Hashable, public utl::Dumpable, public utl::Loggable {

public:
    
    /// The location of this file (may be empty)
    fs::path path;

    /// The raw data of this file
    utl::Buffer<u8> data;
    
    
    //
    // Initializing
    //
    
public:

    virtual ~AnyFile() = default;

    /// Initializes the file with an empty buffer of the given capacity
    void init(isize len);
    /// Initializes the file with the contents of a memory region
    void init(const u8 *buf, isize len);
    /// Initializes the file with the contents of a buffer
    void init(const utl::Buffer<u8> &buffer);
    /// Initializes the file with the contents of a string
    void init(const string &str);
    /// Initializes the file with the contents of a file
    void init(const fs::path &path);


    //
    // Methods from Hashable
    //

public:

    /// Returns a fingerprint (hash value) for this file
    u64 hash(HashAlgorithm algorithm) const override {
        return data.hash(algorithm);
    }


    //
    // Methods from Dumpable
    //

public:

    /// Returns a data provider for dumping the file contents
    Dumpable::DataProvider dataProvider() const override {
        return data.dataProvider();
    }


    //
    // Querying meta information
    //

public:

    /// Returns the size of this file
    isize getSize() const { return data.size; }
    /// Returns a pointer to the file data
    u8* getData() const { return data.ptr; }
    /// Returns true if the file contains no data
    bool empty() const { return data.empty(); }

    /// Returns meta-information about the file
    virtual std::vector<string> describe() const { return {}; }


    //
    // Accessing data
    //

public:

    /// Returns a read-only view of the file contents, starting at an offset
    utl::ByteView byteView(isize offset = 0) const;
    /// Returns a read-only view of a portion of the file contents
    utl::ByteView byteView(isize offset, isize len) const;
    /// Returns a mutable view of the file contents, starting at an offset
    utl::MutableByteView byteView(isize offset = 0);
    /// Returns a mutable view of a portion of the file contents
    utl::MutableByteView byteView(isize offset, isize len);

    // Copies the file contents into a buffer
    /// Copies a portion of the file contents into a buffer
    virtual void copy(u8 *dst, isize offset, isize len) const;
    /// Copies the file contents (starting at an offset) into a buffer
    virtual void copy(u8 *dst, isize offset = 0) const;


    //
    // Importing
    //

    /// Returns true if path points to a compatible file
    virtual bool isCompatiblePath(const fs::path &path) const { return true; }


    //
    // Exporting
    //
    
public:

    /// Writes the file to a stream
    isize writeToStream(std::ostream &stream) const;
    /// Writes the file to a file on disk
    isize writeToFile(const fs::path &path) const;

    /// Writes a portion of the file to a stream
    isize writeToStream(std::ostream &stream, isize offset, isize len) const;
    /// Writes a portion of the file to a file on disk
    isize writeToFile(const fs::path &path, isize offset, isize len) const;

private:
    
    // Called at the end of init()
    virtual void didLoad() {};
};

}
