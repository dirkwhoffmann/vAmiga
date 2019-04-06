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
    
    /* MFM encoded disk data
     *
     * Cylinder  Track     Head      Sectors
     * ---------------------------------------
     * 0         0         0          0 - 10
     * 0         1         1         11 - 21
     * 1         2         0         22 - 32
     * 1         3         1         33 - 43
     *                ...
     * 79        158       0       1738 - 1748
     * 79        159       1       1749 - 1759
     */
    
    static const int numHeads           = 2;
    static const int numCylinders       = 80;
    static const int numSectorsPerTrack = 11;
    static const int numTracks          = numHeads * numCylinders;
    static const int numSectors         = numSectorsPerTrack * numTracks;
   
    /* A single sector consists of
     *    - A sector header build up from 64 MFM bytes.
     *    - 512 bytes of data (1024 MFM bytes).
     *    Hence,
     *    - a sector consists of 64 + 2*512 = 1088 MFM bytes.
     *
     * A single track consists of
     *    - 11 * 1088 = 11.968 MFM bytes.
     *    - A track gap of about 696 MFM bytes (varies with drive speed).
     *    Hence,
     *    - a track usually occupies 11.968 + 696 = 12.664 MFM bytes.
     *    - a cylinder usually occupies 25.328 MFM bytes.
     *    - a disk usually occupies 80 * 2 * 12.664 =  2.026.240 MFM bytes
     */
    
    static const long mfmBytesPerSector   = 1088;
    static const long mfmBytesInTrackGap  = 696;
    static const long mfmBytesPerTrack    = 12664;
    static const long mfmBytesPerCylinder = 25328;
    static const long mfmBytesPerDisk     = 2026240; 
    
    static const uint64_t MFM_DATA_BIT_MASK8  = 0x55;
    static const uint64_t MFM_CLOCK_BIT_MASK8 = 0xAA;

    // MFM encoded disk data
    union {
        uint8_t raw[mfmBytesPerDisk];
        uint8_t track[160][mfmBytesPerTrack];
        uint8_t cyclinder[80][2][mfmBytesPerTrack];
    } data;
    
    bool writeProtected;
    bool modified;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Disk();
    
    // Factory method
    static Disk *makeWithFile(ADFFile *file);
  

public:
    
    
    //
    // Accessing properties
    //
    
    bool isWriteProtected() { return writeProtected; }
    void setWriteProtection(bool value) { writeProtected = value; }
    
    bool isModified() { return modified; }
    void setModified(bool value) { modified = value; }
    
    
    //
    // Handling MFM encoded data
    //
    
private:
    
    // Adds the clock bits to a byte
    uint8_t addClockBits(uint8_t value, uint8_t previous);
    
    
    //
    // Encoding and decoding
    //
    
public:
    
    // Clears the whole disk
    void clearDisk();

    // Clears a single track
    void clearTrack(Track t);

    /* Encodes the whole disk
     */
    bool encodeDisk(ADFFile *adf);
     
    /* Encodes a single track
     */
    bool encodeTrack(ADFFile *adf, Track t);
    
    /* Encodes a single sector
     */
    bool encodeSector(ADFFile *adf, Track t, Sector s);
    
    /* Encodes a certain number of bytes in odd / even format
     */
    void encodeOddEven(uint8_t *target, uint8_t *source, size_t count);
};

#endif
