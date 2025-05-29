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
#include "MediaFile.h"
#include "Checksum.h"
#include "IOUtils.h"
#include "Buffer.h"
#include <sstream>
#include <fstream>

namespace vamiga {

using util::Buffer;

class AnyFile : public CoreObject, public MediaFile {

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

    // void init(std::istream &stream) throws;
    // void init(const fs::path &path, std::istream &stream) throws;
    void init(isize len) throws;
    void init(const u8 *buf, isize len) throws;
    void init(const Buffer<u8> &buffer) throws;
    void init(const string &str) throws;
    void init(const fs::path &path) throws;
    // void init(FILE *file) throws;
    
    explicit operator bool() const { return data.ptr != nullptr; }

    
    //
    // Methods from CoreObject
    //
    
    /*
private:
    
    void _dump(Category category, std::ostream &os) const override { }
     */


    //
    // Methods from MediaFile
    //
    
public:

    virtual isize getSize() const override { return data.size; }
    virtual u8 *getData() const override { return data.ptr; }
    virtual u64 fnv64() const override { return data.fnv64(); }
    virtual u32 crc32() const override { return data.crc32(); }

    
    //
    // Flashing
    //

    // Copies the file contents into a buffer
    virtual void flash(u8 *buf, isize offset, isize len) const override;
    virtual void flash(u8 *buf, isize offset = 0) const override;

    
    //
    // Serializing
    //
    
protected:
    
    virtual bool isCompatiblePath(const fs::path &path) const = 0;
    virtual bool isCompatibleBuffer(const u8 *buf, isize len) = 0;
    bool isCompatibleBuffer(const Buffer<u8> &buffer);
    isize readFromBuffer(const u8 *buf, isize len) throws override;
    isize readFromBuffer(const Buffer<u8> &buffer) throws;

public:
    
    isize writeToStream(std::ostream &stream, isize offset, isize len) const throws;
    isize writeToFile(const fs::path &path, isize offset, isize len) const throws;
    isize writeToBuffer(u8 *buf, isize offset, isize len) const throws;
    isize writeToBuffer(Buffer<u8> &buffer, isize offset, isize len) const throws;

    isize writeToStream(std::ostream &stream) const throws override;
    isize writeToFile(const fs::path &path) const throws override;
    isize writePartitionToFile(const fs::path &path, isize partition) const throws override;
    isize writeToBuffer(u8 *buf) const throws override;
    isize writeToBuffer(Buffer<u8> &buffer) const throws;

private:
    
    // Delegation methods
    virtual void finalizeRead() throws { };
};

}
