// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

bool
AmigaSnapshot::isSnapshot(const uint8_t *buffer, size_t length)
{
    uint8_t signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    assert(buffer != NULL);
    
    if (length < sizeof(AmigaSnapshotHeader)) return false;
    return matchingBufferHeader(buffer, signature, sizeof(signature));
}

bool
AmigaSnapshot::isSnapshot(const uint8_t *buffer, size_t length,
                          uint8_t major, uint8_t minor, uint8_t subminor)
{
    if (!isSnapshot(buffer, length)) return false;
    return buffer[6] == major && buffer[7] == minor && buffer[8] == subminor;
}

bool
AmigaSnapshot::isSupportedSnapshot(const uint8_t *buffer, size_t length)
{
    return isSnapshot(buffer, length, V_MAJOR, V_MINOR, V_SUBMINOR);
}

bool
AmigaSnapshot::isUnsupportedSnapshot(const uint8_t *buffer, size_t length)
{
    return isSnapshot(buffer, length) && !isSupportedSnapshot(buffer, length);
}

bool
AmigaSnapshot::isSnapshotFile(const char *path)
{
     uint8_t signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    assert(path != NULL);
    
    return matchingFileHeader(path, signature, sizeof(signature));
}

bool
AmigaSnapshot::isSnapshotFile(const char *path, uint8_t major, uint8_t minor, uint8_t subminor)
{
    uint8_t signature[] = { 'V', 'C', '6', '4', major, minor, subminor };
    
    assert(path != NULL);
    
    return matchingFileHeader(path, signature, sizeof(signature));
}

bool
AmigaSnapshot::isSupportedSnapshotFile(const char *path)
{
    return isSnapshotFile(path, V_MAJOR, V_MINOR, V_SUBMINOR);
}

bool
AmigaSnapshot::isUnsupportedSnapshotFile(const char *path)
{
    return isSnapshotFile(path) && !isSupportedSnapshotFile(path);
}

AmigaSnapshot::AmigaSnapshot()
{
    setDescription("Snapshot");
}

AmigaSnapshot::AmigaSnapshot(size_t capacity)
{
    uint8_t signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    size = capacity + sizeof(AmigaSnapshotHeader);
    data = new uint8_t[size];
    
    AmigaSnapshotHeader *header = (AmigaSnapshotHeader *)data;
    
    for (unsigned i = 0; i < sizeof(signature); i++)
        header->magic[i] = signature[i];
    header->major = V_MAJOR;
    header->minor = V_MINOR;
    header->subminor = V_SUBMINOR;
    header->timestamp = time(NULL);
}

AmigaSnapshot *
AmigaSnapshot::makeWithBuffer(const uint8_t *buffer, size_t length)
{
    AmigaSnapshot *snapshot = NULL;
    
    if (isSnapshot(buffer, length)) {
        
        snapshot = new AmigaSnapshot();
        
        if (!snapshot->readFromBuffer(buffer, length)) {
            delete snapshot;
            return NULL;
        }
    }
    return snapshot;
}

AmigaSnapshot *
AmigaSnapshot::makeWithFile(const char *path)
{
    AmigaSnapshot *snapshot = NULL;
    
    if (isSnapshotFile(path)) {
        
        snapshot = new AmigaSnapshot();
        
        if (!snapshot->readFromFile(path)) {
            delete snapshot;
            return NULL;
        }
    }
    return snapshot;
}

AmigaSnapshot *
AmigaSnapshot::makeWithAmiga(Amiga *amiga)
{
    AmigaSnapshot *snapshot = new AmigaSnapshot(amiga->stateSize());
    
    snapshot->takeScreenshot(amiga);
    uint8_t *ptr = snapshot->getData();
    amiga->saveToBuffer(&ptr);
    
    return snapshot;
}

bool
AmigaSnapshot::bufferHasSameType(const uint8_t* buffer, size_t length)
{
    return AmigaSnapshot::isSnapshot(buffer, length);
}

bool
AmigaSnapshot::fileHasSameType(const char *path)
{
    return AmigaSnapshot::isSnapshotFile(path, V_MAJOR, V_MINOR, V_SUBMINOR);
}

void
AmigaSnapshot::takeScreenshot(Amiga *amiga)
{
    AmigaSnapshotHeader *header = (AmigaSnapshotHeader *)data;
    
    unsigned x = 5;
    unsigned y = 2;
    unsigned width = HPIXELS - 10;
    unsigned height = 2*VPIXELS - 4;
   
    header->screenshot.width = width;
    header->screenshot.height = height;
  
    uint32_t *shortFrame = (uint32_t *)amiga->denise.shortFrame;
    uint32_t *longFrame = (uint32_t *)amiga->denise.longFrame;
    uint32_t *target = header->screenshot.screen;
    shortFrame += x + y * HPIXELS;
    longFrame += x + y * HPIXELS;

    for (unsigned i = 0; i < height; i++) {
        if (i % 2) {
            memcpy(target, shortFrame, width * 4);
            shortFrame += HPIXELS;
        } else {
            memcpy(target, longFrame, width * 4);
            longFrame += HPIXELS;
        }
        target += width;
    }
}
