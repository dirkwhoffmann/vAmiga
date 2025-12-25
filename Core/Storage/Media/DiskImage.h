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
#include "BlockDevice.h"

namespace vamiga {

class DiskImage : public AnyFile, public BlockDevice {

public:

    struct CHS {

        isize cylinder;
        isize head;
        isize sector;
    };

    struct TS {

        isize track;
        isize sector;
    };

private:
    
    // Maps a track to its first block
    mutable std::vector<isize> track2block;

    // Sets up the track map
    void buildTrackMap() const;

    // Maps a block to its track
    isize block2track(isize b) const;


    //
    // Querying capacity information
    //

public:

    virtual isize numCyls() const = 0;
    virtual isize numHeads() const = 0;
    virtual isize numSectors(isize t) const = 0;

    isize numSectors(isize c, isize h) const { return numSectors(c * numHeads() + h); }
    isize numTracks() const { return numHeads() * numCyls(); }
    isize numBlocks() const { return capacity(); }
    isize numBytes() const { return capacity() * bsize(); }

    //
    // Mapping [c]ylinders, [h]eads, [s]ectors, [b]locks
    //

public:

    // Translates to cylinder/head/sector format
    CHS chs(isize b) const;
    CHS chs(isize t, isize s) const;
    CHS chs(TS ts) const { return chs(ts.track, ts.sector); }

    // Translates to track/sector format
    TS ts(isize b) const;
    TS ts(isize c, isize h, isize s) const;
    TS ts(CHS chs) const { return ts(chs.cylinder, chs.head, chs.sector); }

    // Translates to block numbers
    isize bindex(CHS chs) const;
    isize bindex(TS ts) const;

    // Translates to byte offsets
    isize boffset(CHS chs) const { return bindex(chs) * bsize(); }
    isize boffset(TS ts) const { return bindex(ts) * bsize(); }


    //
    // Methods from TrackDevice
    //
    
public:

    isize bsize() const override { return 512; }
    isize capacity() const override { return data.size / bsize(); }
    void readBlock(u8 *dst, isize nr) override;
    void writeBlock(const u8 *src, isize nr) override;


    // Reads a single byte
    [[deprecated]] virtual u8 readByte(isize b, isize offset) const; // TODO: MOVE to BlockDevice
    [[deprecated]] virtual u8 readByte(isize t, isize s, isize offset) const;

    // Reads a single sector
    // [[deprecated]] virtual void readSector(u8 *dst, isize b) const;
    [[deprecated]] virtual void readSector(u8 *dst, isize t, isize s) const;

    // Write a single byte
    [[deprecated]] virtual void writeByte(isize b, isize offset, u8 value);
    [[deprecated]] virtual void writeByte(isize t, isize s, isize offset, u8 value);

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
