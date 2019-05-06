// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

/*
void printBin(uint64_t value) {

    for (int i = 63; i >= 0; i--) {
        printf("%d", (value & ((uint64_t)1 << i)) != 0);
    }
    printf("  %p\n", (void *)value);
}
*/

Disk::Disk()
{
    setDescription("Disk");
    
    writeProtected = false;
    modified = false;
    clearDisk();
}

Disk *
Disk::makeWithFile(ADFFile *file)
{
    Disk *disk = new Disk();
    
    if (!disk->encodeDisk(file)) {
        delete disk;
        return NULL;
    }
    
    return disk;
}

uint8_t
Disk::readHead(uint8_t cylinder, uint8_t side, uint16_t offset)
{
    assert(cylinder < 80);
    assert(side < 2);
    assert(offset < mfmBytesPerTrack);

    return data.cyclinder[cylinder][side][offset];
}

void
Disk::writeHead(uint8_t value, uint8_t cylinder, uint8_t side, uint16_t offset)
{
    assert(cylinder < 80);
    assert(side < 2);
    assert(offset < mfmBytesPerTrack);
    
    data.cyclinder[cylinder][side][offset] = value;
}

uint8_t
Disk::addClockBits(uint8_t value, uint8_t previous)
{
    // Clear all previously set clock bits
    value &= 0x55;

    // Compute clock bits (clock bit values are inverted)
    uint8_t lShifted = (value << 1);
    uint8_t rShifted = (value >> 1) | (previous << 7);
    uint8_t cBitsInv = lShifted | rShifted;

    // Reverse the computed clock bits
    uint64_t cBits = cBitsInv ^ 0xAA;
    
    // Return original value with the clock bits added
    return value | cBits;
}

void
Disk::clearDisk()
{
    memset(data.raw, 0xAA, mfmBytesPerDisk);
}

void
Disk::clearTrack(Track t)
{
    assert(isTrackNumber(t));
    memset(data.track[t], 0xAA, mfmBytesPerTrack);
}

bool
Disk::encodeDisk(ADFFile *adf)
{
    bool result = true;
    
    debug("Encoding disk...\n");
    
    for (Track t = 0; t < numTracks; t++) {
        result &= encodeTrack(adf, t);
    }

    return result;
}

bool
Disk::encodeTrack(ADFFile *adf, Track t)
{
    assert(isTrackNumber(t));
 
    bool result = true;
    
    debug(2, "Encoding track %d\n", t);
    
    // Remove previously written data
    clearTrack(t);
    
    // Encode each sector
    for (Sector s = 0; s < 11; s++) {
        result &= encodeSector(adf, t, s);
    }
    
    // Get the clock bit right at offset position 0
    if (data.track[t][mfmBytesPerTrack - 1] & 1) data.track[t][0] &= 0x7F;
    
    // First five bytes of track gap
    /*
    uint8_t *p = data.track[t] + (11 * mfmBytesPerSector);
    p[0] = addClockBits(0, p[-1]);
    p[1] = 0xA8;
    p[2] = 0x55;
    p[3] = 0x55;
    p[4] = 0xAA;
    */
    
     if (1) {
 
         uint8_t *p = data.track[t] + (0 * mfmBytesPerSector);
         
         uint32_t check = fnv_1a_init32();
         for (unsigned i = 0; i < 2*6334; i+=2) {
             check = fnv_1a_it32(check, HI_LO(p[i],p[i+1]));
         }
         /*
         for (unsigned i = 0, j = 1; i < 2*2048; i += 16, j++) {
             plaindebug("%2d: %X%X %X%X %X%X %X%X %X%X %X%X %X%X %X%X\n", j,
                        p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7],
                        p[i+8], p[i+9], p[i+10], p[i+11], p[i+12], p[i+13], p[i+14], p[i+15]);
         }
        */
         plaindebug(2, "Track %d checksum = %X\n", t, check);
     }
    
    return result;
}

bool
Disk::encodeSector(ADFFile *adf, Track t, Sector s)
{
    assert(isTrackNumber(t));
    assert(isSectorNumber(s));
    
    debug(2, "Encoding sector %d\n", s);
    
    /* Block header layout:
     *                     Start  Size   Value
     * Bytes before SYNC   00      4     0xAA 0xAA 0xAA 0xAA
     * SYNC mark           04      4     0x44 0x89 0x44 0x89
     * Track & sector info 08      8     Odd/Even encoded
     * Unused area         16     32     0xAA
     * Block checksum      48      8     Odd/Even encoded
     * Data checksum       56      8     Odd/Even encoded
     */
    
    uint8_t *p = data.track[t] + (s * mfmBytesPerSector) + mfmBytesInTrackGap;
    
    // Bytes before SYNC
    p[0] = (p[-1] & 1) ? 0x2A : 0xAA;
    p[1] = 0xAA;
    p[2] = 0xAA;
    p[3] = 0xAA;
    
    // SYNC mark
    uint16_t sync = 0x4489;
    p[4] = HI_BYTE(sync);
    p[5] = LO_BYTE(sync);
    p[6] = HI_BYTE(sync);
    p[7] = LO_BYTE(sync);
    
    // Track and sector information
    uint8_t info[4] = { 0xFF, (uint8_t)t, (uint8_t)s, (uint8_t)(11 - s) };
    encodeOddEven(&p[8], info, sizeof(info));
    
    // Unused area
    for (unsigned i = 16; i < 48; i++)
    p[i] = 0xAA;
    
    // Data
    uint8_t bytes[512];
    adf->readSector(bytes, t, s);
    encodeOddEven(&p[64], bytes, sizeof(bytes));
    
    // Block checksum
    uint8_t bcheck[4] = { 0, 0, 0, 0 };
    for(unsigned i = 8; i < 48; i += 4) {
        bcheck[0] ^= p[i];
        bcheck[1] ^= p[i+1];
        bcheck[2] ^= p[i+2];
        bcheck[3] ^= p[i+3];
    }
    encodeOddEven(&p[48], bcheck, sizeof(bcheck));
    
    // Data checksum
    uint8_t dcheck[4] = { 0, 0, 0, 0 };
    for(unsigned i = 64; i < 1088; i += 4) {
        dcheck[0] ^= p[i];
        dcheck[1] ^= p[i+1];
        dcheck[2] ^= p[i+2];
        dcheck[3] ^= p[i+3];
    }
    encodeOddEven(&p[56], dcheck, sizeof(bcheck));
    
    // Add clock bits
    for(unsigned i = 8; i < 1088; i ++) {
        p[i] = addClockBits(p[i], p[i-1]);
    }
    
    return true;
}

void
Disk::encodeOddEven(uint8_t *target, uint8_t *source, size_t count)
{
    // Encode odd bits
    for(size_t i = 0; i < count; i++)
        target[i] = (source[i] >> 1) & 0x55;
    
    // Encode even bits
    for(size_t i = 0; i < count; i++)
        target[i + count] = source[i] & 0x55;
}

bool
Disk::decodeDisk(uint8_t *dst)
{
    bool result = true;
    
    debug("Decoding disk...\n");
    
    for (Track t = 0; t < numTracks; t++) {
        result &= decodeTrack(dst, t);
        dst += numSectorsPerTrack * 512;
    }
    
    return result;
}

size_t
Disk::decodeTrack(uint8_t *dst, Track t)
{
    assert(isTrackNumber(t));
    
    uint8_t *oldDst = dst;
    
    debug(1, "Decoding track %d (%p)\n", t, dst);
    
    // Create a local (double) copy of the track to easy analysis
    uint8_t local[2 * mfmBytesPerTrack];
    memcpy(local, data.track[t], mfmBytesPerTrack);
    memcpy(local + mfmBytesPerTrack, data.track[t], mfmBytesPerTrack);
    
    // Seek all sync marks
    int sectorStart[11], index = 0, nr = 0;
    while (index < mfmBytesPerTrack + mfmBytesPerSector && nr < 11) {

        if (local[index++] != 0x44) continue;
        if (local[index++] != 0x89) continue;
        if (local[index++] != 0x44) continue;
        if (local[index++] != 0x89) continue;
        
        sectorStart[nr++] = index;
        debug("   Sector %d starts at index %d\n", nr-1, sectorStart[nr-1]);
    }
    
    if (nr != 11) {
        warn("Track %d: Found %d sectors, expected %d. Aborting.\n", t, nr, 11);
        return false;
    }
    
    // Encode each sector
    for (Sector s = 0; s < 11; s++) {
        decodeSector(dst, local + sectorStart[s]);
        dst += 512;
    }
    
    debug("Decoded %d bytes\n", dst - oldDst);
    return true;
}

void
Disk::decodeSector(uint8_t *dst, uint8_t *src)
{
    assert(dst != NULL);
    assert(src != NULL);

    // Skip sector header
    src += 56;
    
    // Decode sector data
    decodeOddEven(dst, src, 512);
}

void
Disk::decodeOddEven(uint8_t *dst, uint8_t *src, size_t count)
{
    // Decode odd bits
    for(size_t i = 0; i < count; i++)
        dst[i] = (src[i] & 0x55) << 1;
    
    // Decode even bits
    for(size_t i = 0; i < count; i++)
        dst[i] |= src[i + count] & 0x55;
}
