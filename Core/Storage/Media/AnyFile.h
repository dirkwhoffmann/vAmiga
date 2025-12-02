// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MediaFileTypes.h"
#include "Annotations.h"
#include "Loggable.h"
#include "Error.h"
#include "Dumpable.h"
#include "Checksum.h"
#include "IOUtils.h"
#include "Buffer.h"
#include <sstream>
#include <fstream>

namespace vamiga {

using util::Buffer;

class AnyFile : public Loggable { // CoreObject {

public:
    
    // Physical location of this file
    fs::path path;

    // The raw data of this file
    Buffer<u8> data;
    
    
    //
    // Initializing
    //
    
public:

    virtual ~AnyFile();

    void init(isize len) throws;
    void init(const u8 *buf, isize len) throws;
    void init(const Buffer<u8> &buffer) throws;
    void init(const string &str) throws;
    void init(const fs::path &path) throws;

    explicit operator bool() const { return data.ptr != nullptr; }


    //
    // General
    //

    // Returns the media type of this file
    virtual FileType type() const { return FileType::UNKNOWN; }

    
    //
    // Data management
    //
    
public:

    virtual isize getSize() const { return data.size; }
    virtual u8 *getData() const { return data.ptr; }
    virtual u64 fnv64() const { return data.fnv64(); }
    virtual u32 crc32() const { return data.crc32(); }

    string getSizeAsString() const;

    
    //
    // Flashing
    //

    // Copies the file contents into a buffer
    virtual void flash(u8 *buf, isize offset, isize len) const;
    virtual void flash(u8 *buf, isize offset = 0) const;


    //
    // Serializing
    //
    
public:

    virtual bool isCompatiblePath(const fs::path &path) const = 0;
    virtual bool isCompatibleBuffer(const u8 *buf, isize len) const = 0;
    bool isCompatibleBuffer(const Buffer<u8> &buffer);
    isize readFromBuffer(const u8 *buf, isize len);
    isize readFromBuffer(const Buffer<u8> &buffer);
    
    isize writeToStream(std::ostream &stream, isize offset, isize len) const;
    isize writeToFile(const fs::path &path, isize offset, isize len) const;
    isize writeToBuffer(u8 *buf, isize offset, isize len) const;
    isize writeToBuffer(Buffer<u8> &buffer, isize offset, isize len) const;

    isize writeToStream(std::ostream &stream) const;
    isize writeToFile(const fs::path &path) const;
    isize writePartitionToFile(const fs::path &path, isize partition) const;
    isize writeToBuffer(u8 *buf) const throws;
    isize writeToBuffer(Buffer<u8> &buffer) const throws;

private:
    
    // Delegation methods
    virtual void finalizeRead() throws { };
};

}
