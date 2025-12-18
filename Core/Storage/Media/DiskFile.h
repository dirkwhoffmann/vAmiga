// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AnyFile.h"
#include "BlockDevice.h"

namespace vamiga {

class DiskFile : public AnyFile, public BlockView {

    //
    // Methods from BlockDevice
    //

public:

    isize capacity() const override { return numBlocks(); }
    isize bsize() const override { return 512; }
    void freeBlock(isize nr) override { }
    Buffer<u8> *readBlock(isize nr) override {

        // REMOVE MEMORY LEAK AFTER TESTING
        auto buf = new Buffer<u8>(bsize());
        readSector(buf->ptr, nr);
        return buf;
    }
    Buffer<u8> *ensureBlock(isize nr) override { return readBlock(nr); }
    void writeBlock(isize nr, const Buffer<u8> &buffer) override {

        assert(buffer.size == bsize());
        writeSector(nr, buffer);
    }

    //
    // Querying disk properties
    //
    
public:

    virtual isize numCyls() const = 0;
    virtual isize numHeads() const = 0;
    virtual isize numSectors() const = 0;
    // isize bsize() const { return 512; }
    isize numTracks() const { return numHeads() * numCyls(); }
    isize numBlocks() const { return numTracks() * numSectors(); }
    isize numBytes() const { return numBlocks() * bsize(); }

    
    //
    // Reading data
    //
    
public:

    // Reads a single byte
    virtual u8 readByte(isize b, isize offset) const;
    virtual u8 readByte(isize t, isize s, isize offset) const;

    // Reads a single sector
    virtual void readSector(u8 *dst, isize b) const;
    virtual void readSector(u8 *dst, isize t, isize s) const;

    // Write a single byte
    virtual void writeByte(isize b, isize offset, u8 value);
    virtual void writeByte(isize t, isize s, isize offset, u8 value);

    // Writes a single sector
    virtual void writeSector(isize b, const Buffer<u8> &buffer);
    virtual void writeSector(isize t, isize s, const Buffer<u8> &buffer);


    //
    // Pretty-printing
    //
    
public:
    
    // Returns a textual description for certain disk properties
    string describeGeometry();
    string describeCapacity();
    
    // Generates a hex dump for some sector data
    string hexdump(isize b, isize offset, isize len) const;
    string hexdump(isize t, isize s, isize offset, isize len) const;
    string hexdump(isize c, isize h, isize s, isize offset, isize len) const;

    // Generates an ASCII dump for some sector data
    string asciidump(isize b, isize offset, isize len) const;
    string asciidump(isize t, isize s, isize offset, isize len) const;
    string asciidump(isize c, isize h, isize s, isize offset, isize len) const;
};

}
