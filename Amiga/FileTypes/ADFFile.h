// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _ADF_FILE_INC
#define _ADF_FILE_INC

#include "AmigaFile.h"

#define ADFSIZE_35_DD     901120  //  880 KB
#define ADFSIZE_35_DD_PC  737280  //  720 KB
#define ADFSIZE_35_HD    1802240  // 1760 KB
#define ADFSIZE_35_HD_PC 1474560  // 1440 KB
#define ADFSIZE_525_SD    368640  //  360 KB

static inline bool isCylinderNr(long nr) { return nr >= 0 && nr <= 79; }
static inline bool isTrackNr(long nr)    { return nr >= 0 && nr <= 159; }
static inline bool isSectorNr(long nr)   { return nr >= 0 && nr <= 1759; }

class ADFFile : public AmigaFile {
    
public:
    
    //
    // Class methods
    //
    
    // Returns true iff buffer contains an ADF file.
    static bool isADFBuffer(const uint8_t *buffer, size_t length);
    
    // Returns true iff path points to an ADF file.
    static bool isADFFile(const char *path);
    
    // Returns the ADF file size for a given disk type.
    static size_t fileSize(DiskType t);

    
    //
    // Creating and destructing
    //
    
    ADFFile();
    
    
    //
    // Factory methods
    //
    
public:
    
    static ADFFile *makeWithDiskType(DiskType t);
    static ADFFile *makeWithBuffer(const uint8_t *buffer, size_t length);
    static ADFFile *makeWithFile(const char *path);
    // static ADFFile *makeUnformatted(DiskType type); // DEPRECATED
    // static ADFFile *makeFormatted(DiskType type, FileSystemType fs); // DEPRECATED

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    AmigaFileType type() override { return FILETYPE_ADF; }
    const char *typeAsString() override { return "ADF"; }
    bool bufferHasSameType(const uint8_t *buffer, size_t length) override {
        return isADFBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isADFFile(path); }
    bool readFromBuffer(const uint8_t *buffer, size_t length) override;
    
    
    //
    // Properties
    //
    
    // Determines the type of this disk
    DiskType getType();
    
    
    //
    // Formatting
    //
 
    void formatDisk(FileSystemType fs);
    
private:
    
    void writeBootBlock(FileSystemType fs);
    void writeRootBlock(uint32_t blockIndex, const char *label);
    void writeBitmapBlock(uint32_t blockIndex, uint32_t numSectors);
    void writeDate(int offset, time_t date);

    uint32_t sectorChecksum(int sector);

    
    //
    // Seeking tracks and sectors
    //
    
public:
    
    /* Prepares to read a track.
     * Use read() to read from the selected track. Returns EOF when the whole
     * track has been read in.
     */
    void seekTrack(long t);
    
    /* Prepares to read a sector.
     * Use read() to read from the selected sector. Returns EOF when the whole
     * sector has been read in.
     */
    void seekSector(long s);

    /* Prepares to read a sector.
     * Use read() to read from the selected track. Returns EOF when the whole
     * track has been read in.
     */
    void seekTrackAndSector(long t, long s) { seekSector(11 * t + s); }
    
    // Fills a buffer with the data of a single sector
    void readSector(uint8_t *target, long t, long s); 
};

#endif
