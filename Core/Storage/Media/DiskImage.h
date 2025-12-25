// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskImageTypes.h"
#include "AnyFile.h"
#include "TrackDevice.h"

namespace vamiga {

class DiskImage : public AnyFile, public TrackDevice {

    //
    // Methods from LinearDevice
    //

    isize size() const override { return data.size; }
    void read(u8 *dst, isize offset, isize count) override;
    void write(const u8 *src, isize offset, isize count) override;


    //
    // Methods from BlockDevice
    //
    
public:

    isize bsize() const override { return 512; }


    //
    // Methods from TrackDevice
    //


    // Reads a single sector
    // [[deprecated]] virtual void readSector(u8 *dst, isize b) const;
    [[deprecated]] virtual void readSector(u8 *dst, isize t, isize s) const;

    // Write a single byte
    // [[deprecated]] virtual void writeByte(isize b, isize offset, u8 value);
    // [[deprecated]] virtual void writeByte(isize t, isize s, isize offset, u8 value);

    // Writes a single sector
    // [[deprecated]] virtual void writeSector(isize b, const Buffer<u8> &buffer);
    // [[deprecated]] virtual void writeSector(isize t, isize s, const Buffer<u8> &buffer);


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
