// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

IMGFile::IMGFile()
{
    setDescription("IMGFile");
}

bool
IMGFile::isIMGBuffer(const u8 *buffer, size_t length)
{
    // There are no magic bytes. We can only check the buffer size
    return
    length == IMGSIZE_35_DD;
}

bool
IMGFile::isIMGFile(const char *path)
{
    // There are no magic bytes. We can only check the file size
    return
    checkFileSize(path, IMGSIZE_35_DD);
}

IMGFile *
IMGFile::makeWithDiskType(DiskType t)
{
    assert(t == DISK_35_DD);
    
    IMGFile *img = new IMGFile();
    
    if (!img->alloc(9 * 160 * 512)) {
        delete img;
        return NULL;
    }
    
    memset(img->data, 0, img->size);
    return img;
}

IMGFile *
IMGFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    IMGFile *img = new IMGFile();
    
    if (!img->readFromBuffer(buffer, length)) {
        delete img;
        return NULL;
    }
    
    return img;
}

IMGFile *
IMGFile::makeWithFile(const char *path)
{
    IMGFile *img = new IMGFile();
    
    if (!img->readFromFile(path)) {
        delete img;
        return NULL;
    }
    
    return img;
}

IMGFile *
IMGFile::makeWithFile(FILE *file)
{
    IMGFile *img = new IMGFile();
    
    if (!img->readFromFile(file)) {
        delete img;
        return NULL;
    }
    
    return img;
}

IMGFile *
IMGFile::makeWithDisk(Disk *disk)
{
    assert(disk != NULL);
    
    // We only support 3.5"DD disks at the moment
    if (disk->getType() != DISK_35_DD) { return NULL; }
    
    IMGFile *img = makeWithDiskType(DISK_35_DD);
    
    if (img) {
        if (!disk->decodeDOSDisk(img->data, 160, 9)) {
            printf("Failed to decode DOS disk\n");
            delete img;
            return NULL;
        }
    }
    
    return img;
}

bool
IMGFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isIMGBuffer(buffer, length);
}

DiskType
IMGFile::getDiskType()
{
    return DISK_35_DD_PC;
}

long
IMGFile::numSides()
{
    return 2;
}

long
IMGFile::numCyclinders()
{
    return 80;
}

long
IMGFile::numSectorsPerTrack()
{
    return 9;
}

bool
IMGFile::encodeMFM(Disk *disk)
{
    bool result = true;
    long tracks = numTracks();
    
    debug(MFM_DEBUG, "Encoding DOS disk with %d tracks\n", tracks);
    
    // Encode all tracks
    for (Track t = 0; t < tracks; t++) result &= encodeMFM(disk, t);
    
    // In debug mode, also run the decoder
    if (MFM_DEBUG) {
        debug("DOS disk fully encoded (success = %d)\n", result);
        IMGFile *tmp = IMGFile::makeWithDisk(disk);
        if (tmp) {
            msg("Decoded image written to /tmp/debug.img\n");
            tmp->writeToFile("/tmp/tmp.img");
        }
    }

    return result;
}

bool
IMGFile::encodeMFM(Disk *disk, Track t)
{
    long sectors = numSectorsPerTrack();

    debug(MFM_DEBUG, "Encoding DOS track %d with %d sectors\n", t, sectors);

    u8 *p = disk->ptr(t);
    // u8 *p = data.track[t];

    // Clear track
    disk->clearTrack(t, 0x92, 0x54);

    // Encode track header
    p += 82;                                        // GAP
    for (int i = 0; i < 24; i++) { p[i] = 0xAA; }   // SYNC
    p += 24;
    p[0] = 0x52; p[1] = 0x24;                       // IAM
    p[2] = 0x52; p[3] = 0x24;
    p[4] = 0x52; p[5] = 0x24;
    p[6] = 0x55; p[7] = 0x52;
    p += 8;
    p += 80;                                        // GAP
        
    // Encode all sectors
    bool result = true;
    for (Sector s = 0; s < sectors; s++) result &= encodeMFM(disk, t, s);
    
    // Compute a checksum for debugging
    if (MFM_DEBUG) {
        u64 check = fnv_1a_32(disk->ptr(t), disk->geometry.trackSize);
        debug("Track %d checksum = %x\n", t, check);
    }

    return result;
}

bool
IMGFile::encodeMFM(Disk *disk, Track t, Sector s)
{
    u8 buf[60 + 512 + 2 + 109]; // Header + Data + CRC + Gap
        
    debug(MFM_DEBUG, "  Encoding DOS sector %d\n", s);
    
    // Write SYNC
    for (int i = 0; i < 12; i++) { buf[i] = 0x00; }
    
    // Write IDAM
    buf[12] = 0xA1;
    buf[13] = 0xA1;
    buf[14] = 0xA1;
    buf[15] = 0xFE;
    
    // Write CHRN
    buf[16] = (u8)(t / 2);
    buf[17] = (u8)(t % 2);
    buf[18] = (u8)(s + 1);
    buf[19] = 2;
    
    // Compute and write CRC
    u16 crc = crc16(&buf[12], 8);
    buf[20] = HI_BYTE(crc);
    buf[21] = LO_BYTE(crc);

    // Write GAP
    for (int i = 22; i < 44; i++) { buf[i] = 0x4E; }

    // Write SYNC
    for (int i = 44; i < 56; i++) { buf[i] = 0x00; }

    // Write DATA AM
    buf[56] = 0xA1;
    buf[57] = 0xA1;
    buf[58] = 0xA1;
    buf[59] = 0xFB;

    // Write DATA
    readSector(&buf[60], t, s);
    
    // Compute and write CRC
    crc = crc16(&buf[56], 516);
    buf[572] = HI_BYTE(crc);
    buf[573] = LO_BYTE(crc);

    // Write GAP
    for (int i = 574; i < sizeof(buf); i++) { buf[i] = 0x4E; }

    // Determine the start of this sector inside the current track
    u8 *p = disk->ptr(t, s);
    // u8 *p = data.track[t] + 194 + s * 1300;

    // Create the MFM data stream
    disk->encodeMFM(p, buf, sizeof(buf));
    disk->addClockBits(p, 2 * sizeof(buf));
    
    // Remove certain clock bits in IDAM block
    p[2*12+1] &= 0xDF;
    p[2*13+1] &= 0xDF;
    p[2*14+1] &= 0xDF;
    
    // Remove certain clock bits in DATA AM block
    p[2*56+1] &= 0xDF;
    p[2*57+1] &= 0xDF;
    p[2*58+1] &= 0xDF;

    return true;
}
