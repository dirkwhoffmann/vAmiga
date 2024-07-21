// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "AmigaFile.h"

namespace vamiga {

class DiskFile : public AmigaFile {

    //
    // Querying disk properties
    //
    
public:

    virtual isize numCyls() const = 0;
    virtual isize numHeads() const = 0;
    virtual isize numSectors() const = 0;
    isize bsize() const { return 512; }
    isize numTracks() const { return numHeads() * numCyls(); }
    isize numBlocks() const { return numTracks() * numSectors(); }
    isize numBytes() const { return numBlocks() * bsize(); }

    
    //
    // Reading data
    //
    
public:

    // Reads a single data byte
    virtual u8 readByte(isize b, isize offset) const override;
    virtual u8 readByte(isize t, isize s, isize offset) const override;

    // Fills a buffer with the data of a single sector
    virtual void readSector(u8 *dst, isize b) const override;
    virtual void readSector(u8 *dst, isize t, isize s) const override;
    
    
    //
    // Pretty-printing
    //
    
public:
    
    // Returns a textual description for certain disk properties
    string describeGeometry();
    string describeCapacity();
    
    // Generates a hex dump for some sector data
    string hexdump(isize b, isize offset, isize len) const override;
    string hexdump(isize t, isize s, isize offset, isize len) const override;
    string hexdump(isize c, isize h, isize s, isize offset, isize len) const override;

    // Generates an ASCII dump for some sector data
    string asciidump(isize b, isize offset, isize len) const override;
    string asciidump(isize t, isize s, isize offset, isize len) const override;
    string asciidump(isize c, isize h, isize s, isize offset, isize len) const override;
};

}
