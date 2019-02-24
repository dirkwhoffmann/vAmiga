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

static inline bool isCylinderNumber(long nr) { return nr >= 0 && nr <= 79; }
static inline bool isTrackNumber(long nr)    { return nr >= 0 && nr <= 159; }
static inline bool isSectorNumber(long nr)   { return nr >= 0 && nr <= 1759; }

class ADFFile : public AmigaFile {
    
public:
    
    //
    // Class methods
    //
    
    // Returns true iff buffer contains an ADF file.
    static bool isADFBuffer(const uint8_t *buffer, size_t length);
    
    // Returns true iff path points to an ADF file.
    static bool isADFFile(const char *path);
    
    
    //
    // Creating and destructing
    //
    
    ADFFile();
    
    
    //
    // Factory methods
    //
    
    static ADFFile *make();
    static ADFFile *makeWithBuffer(const uint8_t *buffer, size_t length);
    static ADFFile *makeWithFile(const char *path);

    
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType type() override { return FILETYPE_ADF; }
    const char *typeAsString() override { return "ADF"; }
    bool bufferHasSameType(const uint8_t *buffer, size_t length) override {
        return isADFBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isADFFile(path); }
    bool readFromBuffer(const uint8_t *buffer, size_t length) override;
    
    
    //
    // Seeking tracks and sectors
    //
    
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
