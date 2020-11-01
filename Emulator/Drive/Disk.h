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
#include "DiskGeometry.h"

class Disk : public AmigaObject {
    
    friend class Drive;
    
    //
    // Constants
    //
    static const long sectorSize   = 1088;
    static const long trackGapSize = 700;
    static const long trackSize    = 11 * sectorSize + trackGapSize;
    static const long cylinderSize = 2 * trackSize;
    static const long diskSize     = 84 * cylinderSize;

    static_assert(trackSize == 12668);
    static_assert(cylinderSize == 25336);
    static_assert(diskSize == 2128224);

    // The type of this disk
    DiskType type;
    
    // The geometry of this disk (derived from the disk type in the constructor)
    DiskGeometry geometry;
    
    // MFM encoded disk data
    union {
        u8 raw[diskSize];
        u8 cyclinder[84][2][trackSize];
        u8 track[168][trackSize];
    } data;
    
    // Indicates if this disk is write protected
    bool writeProtected = false;
    
    // Indicates if the disk has been written to
    bool modified = false;
    
    // Checksum of this disk if it was created from an ADF file, 0 otherwise
    u64 fnv = 0;
    
    
    //
    // Constructing and serializing
    //
    
public:
    
    Disk(DiskType type);

    static Disk *makeWithFile(class DiskFile *file);
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
    DiskDensity getDensity();
    
    bool isWriteProtected() { return writeProtected; }
    void setWriteProtection(bool value) { writeProtected = value; }
    
    bool isModified() { return modified; }
    void setModified(bool value) { modified = value; }
    
    u64 getFnv() { return fnv; }
    

    //
    // Reading and writing
    //
    
    // Reads a byte from disk
    u8 readByte(Cylinder cylinder, Side side, u16 offset);

    // Writes a byte to disk
    void writeByte(u8 value, Cylinder cylinder, Side side, u16 offset);

    
    //
    // Erasing disks
    //
    
public:

    // Initializes the disk with random data
    void clearDisk();

    // Initializes a single track with random data or a specific value
    void clearTrack(Track t);
    void clearTrack(Track t, u8 value);

    
    //
    // Encoding
    //
    
public:
    
    // Encodes a disk
    bool encodeDisk(class DiskFile *df);
    
private:
    
    // Encodes a disk, track, or sector in Amiga format
    bool encodeAmigaDisk(class DiskFile *df);
    bool encodeAmigaTrack(class DiskFile *df, Track t);
    bool encodeAmigaSector(class DiskFile *df, Track t, Sector s);

    // Encodes a disk, track, or sector in DOS format
    bool encodeDosDisk(class DiskFile *df);
    bool encodeDosTrack(class DiskFile *df, Track t);
    bool encodeDosSector(class DiskFile *df, Track t, Sector s);

    
    //
    // Decoding
    //

public:
    
    // Decodes a disk, track, or sector in Amiga format
    bool decodeAmigaDisk(u8 *dst, long numTracks, long numSectors);
    bool decodeAmigaTrack(u8 *dst, Track t, long numSectors);
    void decodeAmigaSector(u8 *dst, u8 *src);

    // Decodes a disk, track, or sector in Amiga format
    bool decodeDOSDisk(u8 *dst, long numTracks, long numSectors);
    bool decodeDOSTrack(u8 *dst, Track t, long numSectors);
    void decodeDOSSector(u8 *dst, u8 *src);

    
    //
    // Encoding and decoding MFM data
    //
    
private:
    
    void encodeMFM(u8 *dst, u8 *src, size_t count);
    void decodeMFM(u8 *dst, u8 *src, size_t count);

    void encodeOddEven(u8 *dst, u8 *src, size_t count);
    void decodeOddEven(u8 *dst, u8 *src, size_t count);

    // Adds the MFM clock bits
    void addClockBits(u8 *dst, size_t count);
    u8 addClockBits(u8 value, u8 previous);
};

#endif
