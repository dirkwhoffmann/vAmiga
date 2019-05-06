// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_DISK_INC
#define _AMIGA_DISK_INC

#include "HardwareComponent.h"
#include "ADFFile.h"

// Data type for addressing tracks
typedef int16_t Track;

// Checks if a given number is a valid track number
static inline bool isTrackNumber(int16_t nr) { return nr < 160; }

// Data type for addressing sectors inside a track
typedef int16_t Sector;

// Checks if a given number is a valid sector number
static inline bool isSectorNumber(int16_t nr) { return nr < 11; }


class Disk : public AmigaObject {
    
public:
    
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
     *
     * A single sector consists of
     *    - A sector header build up from 64 MFM bytes.
     *    - 512 bytes of data (1024 MFM bytes).
     *    Hence,
     *    - a sector consists of 64 + 2*512 = 1088 MFM bytes.
     *
     * A single track of a 3.5"DD disk consists
     *    - 11 * 1088 = 11.968 MFM bytes.
     *    - A track gap of about 696 MFM bytes (varies with drive speed).
     *    Hence,
     *    - a track usually occupies 11.968 + 696 = 12.664 MFM bytes.
     *    - a cylinder usually occupies 25.328 MFM bytes.
     *    - a disk usually occupies 80 * 2 * 12.664 =  2.026.240 MFM bytes
     */
    
    static const long sectorSize      = 1088;
    static const long trackGapSize    = 700; // 696;
    static const long maxTrackSize    = 12668; // 12664;
    static const long maxCylinderSize = 2 * maxTrackSize;
    static const long maxDiskSize     = 80 * maxCylinderSize;
    
    // static const uint64_t MFM_DATA_BIT_MASK8  = 0x55;
    // static const uint64_t MFM_CLOCK_BIT_MASK8 = 0xAA;

    // The type of this disk
    DiskType type = DISK_35_DD;
    
    // Length of a single track in bytes
    uint16_t trackLen = maxTrackSize;
    
    // MFM encoded disk data
    union {
        uint8_t raw[maxDiskSize];
        uint8_t cyclinder[80][2][maxTrackSize];
        uint8_t track[160][maxTrackSize];
    } data;
    
    bool writeProtected;
    bool modified;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Disk(DiskType type);
    
    // Factory method
    static Disk *makeWithFile(ADFFile *file);
  

public:
    
    //
    // Accessing properties
    //
    
    DiskType getType() { return type; }
    
    bool isWriteProtected() { return writeProtected; }
    void setWriteProtection(bool value) { writeProtected = value; }
    
    bool isModified() { return modified; }
    void setModified(bool value) { modified = value; }
    
    
    //
    // Computed properties
    //
    
    // Cylinder, track, and sector counts
    long getNumCyclinders();
    long getNumTracks() { return 2 * getNumCyclinders(); }
    long getNumSectorsPerTrack();
    long getNumSectors() { return getNumTracks() * getNumSectorsPerTrack(); }
    
    // Consistency checking
    bool isValidCylinder(long nr) { return nr >= 0 && nr < getNumCyclinders(); }
    bool isTrackNr(long nr) { return nr >= 0 && nr < getNumTracks(); }
    bool isSectorNr(long nr) { return nr >= 0 && nr < getNumSectorsPerTrack(); }
    
    
    //
    // Reading and writing
    //
    
    // Reads a byte from disk
    uint8_t readHead(uint8_t cylinder, uint8_t side, uint16_t offset);

    // Writes a byte to disk
    void writeHead(uint8_t value, uint8_t cylinder, uint8_t side, uint16_t offset);

    
    //
    // Handling MFM encoded data
    //
    
private:
    
    // Adds the clock bits to a byte
    uint8_t addClockBits(uint8_t value, uint8_t previous);
    
    
    //
    // MFM Encoding
    //
    
public:
    
    // Clears the whole disk
    void clearDisk();

    // Clears a single track
    void clearTrack(Track t);

    // Encodes the whole disk
    bool encodeDisk(ADFFile *adf);
    
private:
    
    // Work horses
    bool encodeTrack(ADFFile *adf, Track t, long smax);
    bool encodeSector(ADFFile *adf, Track t, Sector s);
    void encodeOddEven(uint8_t *target, uint8_t *source, size_t count);
    
    
    //
    // MFM Decoding
    //
    
public:
    
    // Decodes the whole disk
    bool decodeDisk(uint8_t *dst);
    
private:
    
    // Work horses
    size_t decodeTrack(uint8_t *dst, Track t, long smax);
    void decodeSector(uint8_t *dst, uint8_t *src);
    void decodeOddEven(uint8_t *dst, uint8_t *src, size_t count);
};

#endif
