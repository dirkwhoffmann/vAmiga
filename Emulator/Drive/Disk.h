// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_DISK_H
#define _AMIGA_DISK_H

#include "HardwareComponent.h"
#include "ADFFile.h"

class Disk : public AmigaObject {
    
    friend class Drive;
    
    //
    // Constants
    //
    
    /*
     * MFM encoded disk data of a standard 3.5"DD disk
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
    static const long sectorSize   = 1088;
    static const long trackGapSize = 700;
    static const long trackSize    = 11 * sectorSize + trackGapSize;
    static const long cylinderSize = 2 * trackSize;
    static const long diskSize     = 84 * cylinderSize;

    static_assert(trackSize == 12668);
    static_assert(cylinderSize == 25336);
    static_assert(diskSize == 2128224);

    // The type of this disk
    DiskType type = DISK_35_DD;
    
    // MFM encoded disk data
    union {
        u8 raw[diskSize];
        u8 cyclinder[84][2][trackSize];
        u8 track[168][trackSize];
    } data;
    
    // Indicates if this disk is write protected
    bool writeProtected;
    
    // Indicates if the disk has been written to
    bool modified;
    
    // Checksum of this disk if it was created from an ADF file, 0 otherwise
    u64 fnv;
    
    
    //
    // Class functions
    //
    
    static long numSides(DiskType type);
    static long numCylinders(DiskType type);
    static long numTracks(DiskType type);
    static long numSectorsPerTrack(DiskType type);
    static long numSectorsTotal(DiskType type);

    
    //
    // Constructing and serializing
    //
    
public:
    
    Disk(DiskType type);

    static Disk *makeWithFile(ADFFile *file);
    static Disk *makeWithReader(SerReader &reader, DiskType diskType);
    
    
    //
    // Serializing
    //

private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & type
        & data.raw
        & writeProtected
        & modified
        & fnv;
    }


    //
    // Accessing
    //

public:

    DiskType getType() { return type; }
    
    bool isWriteProtected() { return writeProtected; }
    void setWriteProtection(bool value) { writeProtected = value; }
    
    bool isModified() { return modified; }
    void setModified(bool value) { modified = value; }
    
    u64 getFnv() { return fnv; }
    
    
    //
    // Computed properties
    //
    
    // Cylinder, track, and sector counts
    long numSides() { return numSides(type); }
    long numCyclinders() { return numCylinders(type); }
    long numTracks() { return numTracks(type); }
    long numSectorsPerTrack() { return numSectorsPerTrack(type); }
    long numSectorsTotal() { return numSectorsTotal(type); }
    
    // Consistency checking
    bool isValidSideNr(Side s) { return s >= 0 && s < numSides(); }
    bool isValidCylinderNr(Cylinder c) { return c >= 0 && c < numCyclinders(); }
    bool isValidTrack(Track t) { return t >= 0 && t < numTracks(); }
    bool isValidSector(Sector s) { return s >= 0 && s < numSectorsPerTrack(); }
    
    
    //
    // Reading and writing
    //
    
    // Reads a byte from disk
    u8 readByte(Cylinder cylinder, Side side, u16 offset);

    // Writes a byte to disk
    void writeByte(u8 value, Cylinder cylinder, Side side, u16 offset);

    
    //
    // Handling MFM encoded data
    //
    
private:
    
    // Adds the clock bits to a byte
    u8 addClockBits(u8 value, u8 previous);
    
    
    //
    // MFM encoding
    //
    
public:
    
    // Initializes the disk with random data
    void clearDisk();

    // Initializes a single track with random data or a specific value
    void clearTrack(Track t);
    void clearTrack(Track t, u8 value);

    // Encodes the whole disk
    bool encodeDisk(ADFFile *adf);
    
private:
    
    // Work horses
    bool encodeTrack(ADFFile *adf, Track t, long smax);
    bool encodeSector(ADFFile *adf, Track t, Sector s);
    void encodeOddEven(u8 *target, u8 *source, size_t count);
    
    
    //
    // MFM decoding
    //
    
public:
    
    // Decodes the whole disk
    bool decodeDisk(u8 *dst, int tracks, int sectors);
    
private:
    
    // Work horses
    bool decodeTrack(u8 *dst, Track t, long smax);
    void decodeSector(u8 *dst, u8 *src);
    void decodeOddEven(u8 *dst, u8 *src, size_t count);
};

#endif
