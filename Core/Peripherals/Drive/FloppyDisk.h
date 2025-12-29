// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FloppyDiskTypes.h"
#include "DriveTypes.h"
#include "CoreComponent.h"
#include "TrackDevice.h"

namespace vamiga {

class FloppyDiskImage;

/* MFM encoded disk data of a standard 3.5" DD disk:
 *
 *    Cylinder  Track     Head      Sectors
 *    ---------------------------------------
 *    0         0         0          0 - 10
 *    0         1         1         11 - 21
 *    1         2         0         22 - 32
 *    1         3         1         33 - 43
 *                   ...
 *    79        158       0       1738 - 1748
 *    79        159       1       1749 - 1759
 *
 *    80        160       0       1760 - 1770   <--- beyond spec
 *    80        161       1       1771 - 1781
 *                   ...
 *    83        166       0       1826 - 1836
 *    83        167       1       1837 - 1847
 *
 * A single sector consists of
 *    - A sector header build up from 64 MFM bytes.
 *    - 512 bytes of data (1024 MFM bytes).
 *
 * Hence,
 *    - a sector consists of 64 + 2*512 = 1088 MFM bytes.
 *
 * A single track of a 3.5"DD disk consists
 *    - 11 * 1088 = 11.968 MFM bytes.
 *    - A track gap of about 700 MFM bytes (varies with drive speed).
 *
 * Hence,
 *    - a track usually occupies 11.968 + 700 = 12.668 MFM bytes.
 *    - a cylinder usually occupies 25.328 MFM bytes.
 *    - a disk usually occupies 84 * 2 * 12.664 =  2.127.552 MFM bytes
 */

class FloppyDisk : public CoreObject, public TrackDevice {

    friend class FloppyDrive;
    friend class ADFEncoder;
    friend class EADFEncoder;
    friend class IMGEncoder;
    friend class STEncoder;
    friend class DiskEncoder;

    friend class EADFFactory;

public:

    // The form factor of this disk
    Diameter diameter;
    
    // The density of this disk
    Density density;
    
private:
    
    // The MFM encoded disk data
    union {
        u8 raw[168*32768];
        u8 cylinder[84][2][32768];
        u8 track[168][32768];
    } data;
    
    // Length of each track in bytes
    [[deprecated]] union {
        i32 cylinder[84][2];
        i32 track[168];
    } length;

    // Experimental
    MutableByteView track[168] {};

    // Disk state
    long flags = 0;
    
    
    //
    // Initializing
    //
    
public:
    
    FloppyDisk() = default;
    FloppyDisk(Diameter dia, Density den, bool wp = false) { init(dia, den, wp); }
    FloppyDisk(const FloppyDiskImage &file, bool wp = false) { init(file, wp); }
    FloppyDisk(SerReader &reader, Diameter dia, Density den, bool wp = false) {
        init(reader, dia, den, wp); }
    ~FloppyDisk();
    
private:
    
    void init(Diameter dia, Density den, bool wp);
    void init(const class FloppyDiskImage &file, bool wp);
    void init(unique_ptr<FloppyDiskImage> file, bool wp);
    void init(SerReader &reader, Diameter dia, Density den, bool wp);

    
public:

    FloppyDisk& operator= (const FloppyDisk& other) {

        CLONE(diameter)
        CLONE(density)
        CLONE_ARRAY(data.raw)
        CLONE_ARRAY(length.track)
        CLONE(flags)

        for (isize i = 0; i < 168; ++i) {
            track[i] = MutableByteView(track[i].data(), other.track[i].size());
        }

        return *this;
    }


    //
    // Methods from CoreObject
    //
    
private:
    
    const char *objectName() const override { return "Disk"; }
    void _dump(Category category, std::ostream &os) const override;


    //
    // Methods from LinearDevice
    //

    isize size() const override { fatalError; }
    void read(u8 *dst, isize offset, isize count) const override { fatalError; }
    void write(const u8 *src, isize offset, isize count) override  { fatalError; }


    //
    // Methods from BlockDevice
    //

public:

    isize capacity() const override { return numCyls() * numHeads() * numSectors(0); }
    isize bsize() const override { return 512; }
    void readBlock(u8 *dst, isize nr) const override;
    void writeBlock(const u8 *src, isize nr) override;


    //
    // Methods from TrackDevice
    //

public:
    
    isize numCyls() const override { return diameter == Diameter::INCH_525 ? 42 : 84; }
    isize numHeads() const override { return 2; }
    isize numSectors(isize t) const override { return density == Density::DD ? 11 : 22; }
    void readTrack(u8 *dst, isize nr) const override;
    void writeTrack(const u8 *src, isize nr) override;


    //
    // Serializing
    //
    
private:
    
    template <class T>
    void serialize(T& worker)
    {
        if (isResetter(worker)) return;

        worker

        << diameter
        << density
        << data.raw
        << length.track
        << flags;
    };

    //
    // Performing sanity checks
    //

    static bool isValidTrackNr(isize value) { return value >= 0 && value < 168; }
    static bool isValidCylinderNr(isize value) { return value >= 0 && value < 84; }
    static bool isValidHeadNr(isize value) { return value >= 0 && value < 2; }
    bool isValidHeadPos(TrackNr t, isize offset) const;
    bool isValidHeadPos(CylNr c, HeadNr h, isize offset) const;

    // Computes a debug checksum for a single track or the entire disk
    u64 checksum() const;
    u64 checksum(TrackNr t) const;
    u64 checksum(CylNr c, HeadNr h) const;


    //
    // Accessing disk parameters
    //
    
public:
    
    Diameter getDiameter() const { return diameter; }
    Density getDensity() const { return density; }

    bool isWriteProtected() const { return flags & long(DiskFlags::PROTECTED); }
    void setWriteProtection(bool value) { value ? flags |= long(DiskFlags::PROTECTED) : flags &= ~long(DiskFlags::PROTECTED); }

    bool isModified() const { return flags & long(DiskFlags::MODIFIED); }
    void setModified(bool value) { value ? flags |= long(DiskFlags::MODIFIED) : flags &= ~long(DiskFlags::MODIFIED); }

    bool getFlag(DiskFlags mask) { return (flags & long(mask)) == long(mask); }
    void setFlag(DiskFlags mask, bool value) { value ? flags |= long(mask) : flags &= ~long(mask); }
    void setFlag(DiskFlags flag) { setFlag(flag, true); }
    void clearFlag(DiskFlags flag) { setFlag(flag, false); }


    //
    // Accessing tracks and sectors
    //

    ByteView byteView(TrackNr t) const;
    ByteView byteView(TrackNr t, SectorNr s) const;
    MutableByteView byteView(TrackNr t);
    MutableByteView byteView(TrackNr t, SectorNr s);



    //
    // Reading and writing
    //

    // Reads a bit from disk
    u8 readBit(TrackNr t, isize offset) const;
    u8 readBit(CylNr c, HeadNr h, isize offset) const;

    // Writes a bit to disk
    void writeBit(TrackNr t, isize offset, bool value);
    void writeBit(CylNr c, HeadNr h, isize offset, bool value);

    // Reads a byte from disk
    u8 read8(TrackNr t, isize offset) const;
    u8 read8(CylNr c, HeadNr h, isize offset) const;
    
    // Writes a byte to disk
    void write8(TrackNr t, isize offset, u8 value);
    void write8(CylNr c, HeadNr h, isize offset, u8 value);
    
    
    //
    // Erasing
    //
    
public:
    
    // Initializes the disk with random data
    void clearDisk();
    
    // Initializes the disk with a constant value
    void clearDisk(u8 value);
    
    // Initializes a single track with random data or a specific value
    void clearTrack(TrackNr t);
    void clearTrack(TrackNr t, u8 value);
    void clearTrack(TrackNr t, u8 value1, u8 value2);
    
    
    //
    // Encoding
    //
    
public:
    
    // Encodes a disk
    void encodeDisk(const class FloppyDiskImage &file);

    // Shifts the tracks agains each other
    void shiftTracks(isize offset);

    
    //
    // Working with MFM encoded data streams
    //
    
public:
    
 
    
    // Repeats the MFM data inside the track buffer to ease decoding (DEPRECATED)
    void repeatTracks();
    
    // Returns a textual representation of all bits of a track
    string readTrackBits(TrackNr t) const;
    string readTrackBits(CylNr c, HeadNr h) const;
};

}
