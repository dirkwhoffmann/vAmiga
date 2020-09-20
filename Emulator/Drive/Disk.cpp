// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Disk::Disk(DiskType type)
{
    setDescription("Disk");
    
    this->type = type;
    writeProtected = false;
    modified = false;
    clearDisk();
}

Disk *
Disk::makeWithFile(DiskFile *file)
{
    Disk *disk = new Disk(file->getDiskType());
    
    if (!disk->encodeDisk(file)) {
        delete disk;
        return NULL;
    }
    
    disk->fnv = file->fnv();
    
    return disk;
}

Disk *
Disk::makeWithReader(SerReader &reader, DiskType diskType)
{
    Disk *disk = new Disk(diskType);
    disk->applyToPersistentItems(reader);
    
    return disk;
}

u8
Disk::readByte(Cylinder cylinder, Side side, u16 offset)
{
    assert(cylinder < 84);
    assert(side < 2);
    assert(offset < trackSize);

    return data.cyclinder[cylinder][side][offset];
}

void
Disk::writeByte(u8 value, Cylinder cylinder, Side side, u16 offset)
{
    assert(cylinder < 84);
    assert(side < 2);
    assert(offset < trackSize);

    data.cyclinder[cylinder][side][offset] = value;
}

void
Disk::clearDisk()
{
    assert(sizeof(data) == sizeof(data.raw));

    srand(0);
    for (int i = 0; i < sizeof(data); i++)
        data.raw[i] = rand() & 0xFF;
    
    /* We are allowed to place random data here. In order to make some copy
     * protected game titles work, we smuggle in some magic values.
     * Crunch factory: Looks for 0x44A2 on cylinder 80
     */
    for (int t = 0; t < 2*84; t++) {
        data.track[t][0] = 0x44;
        data.track[t][1] = 0xA2;
    }
    
    fnv = 0;
}

void
Disk::clearTrack(Track t)
{
    assert(t < 168);

    srand(0);
    for (int i = 0; i < sizeof(data.track[t]); i++)
        data.track[t][i] = rand() & 0xFF;
}

void
Disk::clearTrack(Track t, u8 value)
{
    assert(t < 168);

    memset(data.track[t], value, trackSize);
}

bool
Disk::encodeDisk(DiskFile *df)
{
    assert(df != NULL);
    assert(df->getDiskType() == getType());

    // Start with an unformatted disk
    clearDisk();

    // Call the proper encoder for this disk
    return df->isAmigaDisk() ? encodeAmigaDisk(df) : encodeDosDisk(df);
}

bool
Disk::encodeAmigaDisk(DiskFile *df)
{
    long tracks = df->numTracks();
    
    debug("Encoding Amiga disk (%d tracks)\n", tracks);
    
    bool result = true;
    for (Track t = 0; t < tracks; t++) result &= encodeAmigaTrack(df, t);
    return result;
}

bool
Disk::encodeAmigaTrack(DiskFile *df, Track t)
{
    assert(t < 168);

    long sectors = df->numSectorsPerTrack();
    
    debug(MFM_DEBUG, "Encoding Amiga track %d (%d sectors)\n", t, sectors);

    // Format track
    clearTrack(t, 0xAA);

    // Encode all sectors
    bool result = true;
    for (Sector s = 0; s < sectors; s++) result &= encodeAmigaSector(df, t, s);
    
    // Get the clock bit right at offset position 0
    if (data.track[t][trackSize - 1] & 1) data.track[t][0] &= 0x7F;

    // Compute a debugging checksum
    if (MFM_DEBUG) {
        u64 check = fnv_1a_32(data.track[t], trackSize);
        plaindebug("Track %d checksum = %x\n", t, check);
    }

    return result;
}

bool
Disk::encodeAmigaSector(DiskFile *df, Track t, Sector s)
{
    assert(t < 168);
    assert(s < 11);
    
    debug(MFM_DEBUG, "Encoding sector %d\n", s);
    
    /* Block header layout:
     *                     Start  Size   Value
     * Bytes before SYNC   00      4     0xAA 0xAA 0xAA 0xAA
     * SYNC mark           04      4     0x44 0x89 0x44 0x89
     * Track & sector info 08      8     Odd/Even encoded
     * Unused area         16     32     0xAA
     * Block checksum      48      8     Odd/Even encoded
     * Data checksum       56      8     Odd/Even encoded
     */
    
    u8 *p = data.track[t] + (s * sectorSize) + trackGapSize;
    
    // Bytes before SYNC
    p[0] = (p[-1] & 1) ? 0x2A : 0xAA;
    p[1] = 0xAA;
    p[2] = 0xAA;
    p[3] = 0xAA;
    
    // SYNC mark
    u16 sync = 0x4489;
    p[4] = HI_BYTE(sync);
    p[5] = LO_BYTE(sync);
    p[6] = HI_BYTE(sync);
    p[7] = LO_BYTE(sync);
    
    // Track and sector information
    u8 info[4] = { 0xFF, (u8)t, (u8)s, (u8)(11 - s) };
    encodeOddEven(&p[8], info, sizeof(info));
    
    // Unused area
    for (unsigned i = 16; i < 48; i++)
    p[i] = 0xAA;
    
    // Data
    u8 bytes[512];
    df->readSector(bytes, t, s);
    encodeOddEven(&p[64], bytes, sizeof(bytes));
    
    // Block checksum
    u8 bcheck[4] = { 0, 0, 0, 0 };
    for(unsigned i = 8; i < 48; i += 4) {
        bcheck[0] ^= p[i];
        bcheck[1] ^= p[i+1];
        bcheck[2] ^= p[i+2];
        bcheck[3] ^= p[i+3];
    }
    encodeOddEven(&p[48], bcheck, sizeof(bcheck));
    
    // Data checksum
    u8 dcheck[4] = { 0, 0, 0, 0 };
    for(unsigned i = 64; i < 1088; i += 4) {
        dcheck[0] ^= p[i];
        dcheck[1] ^= p[i+1];
        dcheck[2] ^= p[i+2];
        dcheck[3] ^= p[i+3];
    }
    encodeOddEven(&p[56], dcheck, sizeof(bcheck));
    
    // Add clock bits
    for(unsigned i = 8; i < 1088; i++) {
        p[i] = addClockBits(p[i], p[i-1]);
    }
    
    return true;
}

bool
Disk::encodeDosDisk(DiskFile *df)
{
    long tracks = df->numTracks();
    
    debug(MFM_DEBUG, "Encoding DOS disk (%d tracks)\n", tracks);
    
    bool result = true;
    for (Track t = 0; t < tracks; t++) result &= encodeDosTrack(df, t);
    return result;
}

bool
Disk::encodeDosTrack(DiskFile *df, Track t)
{
    long sectors = df->numSectorsPerTrack();

    debug(MFM_DEBUG, "Encoding DOS track %d (%d sectors)\n", t, sectors);

    assert(false);
    return false;
}

bool
Disk::encodeDosSector(DiskFile *df, Track t, Sector s)
{
    assert(false);
    return false;
}

bool
Disk::decodeAmigaDisk(u8 *dst, int tracks, int sectors)
{
    bool result = true;
        
    debug("Decoding disk (%d tracks, %d sectors each)...\n", tracks, sectors);
    
    for (Track t = 0; t < tracks; t++) {
        result &= decodeAmigaTrack(dst, t, sectors);
        dst += sectors * 512;
    
    }
    
    return result;
}

bool
Disk::decodeAmigaTrack(u8 *dst, Track t, long smax)
{
    assert(t < 168);
        
    debug(MFM_DEBUG, "Decoding track %d\n", t);
    
    // Create a local (double) copy of the track to simply the analysis
    u8 local[2 * trackSize];
    memcpy(local, data.track[t], trackSize);
    memcpy(local + trackSize, data.track[t], trackSize);
    
    // Seek all sync marks
    int sectorStart[smax], index = 0, nr = 0;
    while (index < trackSize + sectorSize && nr < smax) {

        if (local[index++] != 0x44) continue;
        if (local[index++] != 0x89) continue;
        if (local[index++] != 0x44) continue;
        if (local[index++] != 0x89) continue;
        
        sectorStart[nr++] = index;
        // debug("   Sector %d starts at index %d\n", nr-1, sectorStart[nr-1]);
    }
    
    if (nr != smax) {
        warn("Track %d: Found %d sectors, expected %d. Aborting.\n", t, nr, smax);
        return false;
    }
    
    // Encode all sectors
    for (Sector s = 0; s < smax; s++) {
        decodeAmigaSector(dst, local + sectorStart[s]);
        dst += 512;
    }
    
    return true;
}

void
Disk::decodeAmigaSector(u8 *dst, u8 *src)
{
    assert(dst != NULL);
    assert(src != NULL);

    // Skip sector header
    src += 56;
    
    // Decode sector data
    decodeOddEven(dst, src, 512);
}

void
Disk::encodeOddEven(u8 *dst, u8 *src, size_t count)
{
    // Encode odd bits
    for(size_t i = 0; i < count; i++)
        dst[i] = (src[i] >> 1) & 0x55;
    
    // Encode even bits
    for(size_t i = 0; i < count; i++)
        dst[i + count] = src[i] & 0x55;
}

void
Disk::decodeOddEven(u8 *dst, u8 *src, size_t count)
{
    // Decode odd bits
    for(size_t i = 0; i < count; i++)
        dst[i] = (src[i] & 0x55) << 1;
    
    // Decode even bits
    for(size_t i = 0; i < count; i++)
        dst[i] |= src[i + count] & 0x55;
}

u8
Disk::addClockBits(u8 value, u8 previous)
{
    // Clear all previously set clock bits
    value &= 0x55;

    // Compute clock bits (clock bit values are inverted)
    u8 lShifted = (value << 1);
    u8 rShifted = (value >> 1) | (previous << 7);
    u8 cBitsInv = lShifted | rShifted;

    // Reverse the computed clock bits
    u64 cBits = cBitsInv ^ 0xAA;
    
    // Return original value with the clock bits added
    return value | cBits;
}
