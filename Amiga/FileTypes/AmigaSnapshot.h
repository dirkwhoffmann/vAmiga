// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_SNAPSHOT_INC
#define _AMIGA_SNAPSHOT_INC

#include "AmigaFile.h"

class Amiga;

// Snapshot header
typedef struct {
    
    // Magic bytes ('V','A','S','N','A','P')
    char magic[6];
    
    // Version number (V major.minor.subminor)
    uint8_t major;
    uint8_t minor;
    uint8_t subminor;
    
    // Screenshot
    struct {
        
        // Image width and height
        uint16_t width, height;
        
        // Raw screen buffer data
        uint32_t screen[(HPIXELS * (2 * VPIXELS))];
        
    } screenshot;
    
    // Date and time of snapshot creation
    time_t timestamp;
    
} AmigaSnapshotHeader;

class AmigaSnapshot : public AmigaFile {
 
    //
    // Class methods
    //
    
public:
    
    // Returns true iff buffer contains a snapshot.
    static bool isSnapshot(const uint8_t *buffer, size_t length);
    
    // Returns true iff buffer contains a snapshot of a specific version.
    static bool isSnapshot(const uint8_t *buffer, size_t length,
                           uint8_t major, uint8_t minor, uint8_t subminor);
    
    // Returns true iff buffer contains a snapshot with a supported version number.
    static bool isSupportedSnapshot(const uint8_t *buffer, size_t length);
    
    // Returns true iff buffer contains a snapshot with an outdated version number.
    static bool isUnsupportedSnapshot(const uint8_t *buffer, size_t length);
    
    // Returns true if path points to a snapshot file.
    static bool isSnapshotFile(const char *path);
    
    // Returns true if file points to a snapshot file of a specific version.
    static bool isSnapshotFile(const char *path,
                               uint8_t major, uint8_t minor, uint8_t subminor);
    
    // Returns true if file is a snapshot with a supported version number.
    static bool isSupportedSnapshotFile(const char *path);
    
    // Returns true if file is a snapshot with an outdated version number.
    static bool isUnsupportedSnapshotFile(const char *path);
    
    
    //
    // Creating and destructing
    //
    
    AmigaSnapshot();
    AmigaSnapshot(size_t capacity);
    
    // Allocates memory for storing the emulator state.
    bool setCapacity(size_t size);
    
    // Factory methods
    static AmigaSnapshot *makeWithFile(const char *filename);
    static AmigaSnapshot *makeWithBuffer(const uint8_t *buffer, size_t size);
    static AmigaSnapshot *makeWithAmiga(Amiga *amiga);
    
    
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType type() override { return FILETYPE_SNAPSHOT; }
    const char *typeAsString() override { return "VAMIGA"; }
    bool bufferHasSameType(const uint8_t *buffer, size_t length) override;
    bool fileHasSameType(const char *filename) override;
    
    
    //
    // Accessing snapshot properties
    //
    
public:
    
    // Returns pointer to header data
    AmigaSnapshotHeader *getHeader() { return (AmigaSnapshotHeader *)data; }
    
    // Returns pointer to core data
    uint8_t *getData() { return data + sizeof(AmigaSnapshotHeader); }
    
    // Returns the timestamp
    time_t getTimestamp() { return getHeader()->timestamp; }
    
    // Returns a pointer to the screenshot data.
    unsigned char *getImageData() { return (unsigned char *)(getHeader()->screenshot.screen); }
    
    // Returns the screenshot image width
    unsigned getImageWidth() { return getHeader()->screenshot.width; }
    
    // Returns the screenshot image height
    unsigned getImageHeight() { return getHeader()->screenshot.height; }
    
    // Stores a screenshot inside this snapshot
    void takeScreenshot(Amiga *amiga);
    
};

#endif
