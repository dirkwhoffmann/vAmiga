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

class DiskImage : public AnyFile, public BlockDevice {

    //
    // Querying disk capacity
    //
    
public:

    virtual isize numCyls() const = 0;
    virtual isize numHeads() const = 0;
    // virtual isize numSectors(isize track) const = 0;
    virtual isize numSectors() const = 0;
    isize numTracks() const { return numHeads() * numCyls(); }
    isize numBlocks() const { return capacity(); }
    isize numBytes() const { return numBlocks() * bsize(); }


    //
    // Mapping layout parameters ([c]ylinders, [h]eads, [s]ectors)
    //

public:

    virtual isize trackOf(isize c, isize h) { return c * numHeads() + h; }
    virtual isize cylOf(isize t) { return t / numHeads(); }
    virtual isize headOf(isize t) { return t % numHeads(); }


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
