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
Snapshot::isSnapshot(const u8 *buffer, size_t length)
{
    u8 signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    assert(buffer != NULL);
    
    if (length < sizeof(SnapshotHeader)) return false;
    return matchingBufferHeader(buffer, signature, sizeof(signature));
}

bool
Snapshot::isSnapshot(const u8 *buffer, size_t length,
                          u8 major, u8 minor, u8 subminor)
{
    if (!isSnapshot(buffer, length)) return false;
    return buffer[6] == major && buffer[7] == minor && buffer[8] == subminor;
}

bool
Snapshot::isSupportedSnapshot(const u8 *buffer, size_t length)
{
    return isSnapshot(buffer, length, V_MAJOR, V_MINOR, V_SUBMINOR);
}

bool
Snapshot::isUnsupportedSnapshot(const u8 *buffer, size_t length)
{
    return isSnapshot(buffer, length) && !isSupportedSnapshot(buffer, length);
}

bool
Snapshot::isSnapshotFile(const char *path)
{
     u8 signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    assert(path != NULL);
    
    return matchingFileHeader(path, signature, sizeof(signature));
}

bool
Snapshot::isSnapshotFile(const char *path, u8 major, u8 minor, u8 subminor)
{
    u8 signature[] = { 'V', 'C', '6', '4', major, minor, subminor };
    
    assert(path != NULL);
    
    return matchingFileHeader(path, signature, sizeof(signature));
}

bool
Snapshot::isSupportedSnapshotFile(const char *path)
{
    return isSnapshotFile(path, V_MAJOR, V_MINOR, V_SUBMINOR);
}

bool
Snapshot::isUnsupportedSnapshotFile(const char *path)
{
    return isSnapshotFile(path) && !isSupportedSnapshotFile(path);
}

Snapshot::Snapshot()
{
    setDescription("Snapshot");
}

Snapshot::Snapshot(size_t capacity)
{
    u8 signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    size = capacity + sizeof(SnapshotHeader);
    data = new u8[size];
    
    SnapshotHeader *header = (SnapshotHeader *)data;
    
    for (unsigned i = 0; i < sizeof(signature); i++)
        header->magic[i] = signature[i];
    header->major = V_MAJOR;
    header->minor = V_MINOR;
    header->subminor = V_SUBMINOR;
}

Snapshot *
Snapshot::makeWithBuffer(const u8 *buffer, size_t length)
{
    Snapshot *snapshot = NULL;
    
    if (isSnapshot(buffer, length)) {
        
        snapshot = new Snapshot();
        
        if (!snapshot->readFromBuffer(buffer, length)) {
            delete snapshot;
            return NULL;
        }
    }
    return snapshot;
}

Snapshot *
Snapshot::makeWithFile(const char *path)
{
    Snapshot *snapshot = NULL;
    
    if (isSnapshotFile(path)) {
        
        snapshot = new Snapshot();
        
        if (!snapshot->readFromFile(path)) {
            delete snapshot;
            return NULL;
        }
    }
    return snapshot;
}

Snapshot *
Snapshot::makeWithAmiga(Amiga *amiga)
{
    Snapshot *snapshot = new Snapshot(amiga->size());

    snapshot->getHeader()->screenshot.take(amiga);
    amiga->save(snapshot->getData());

    return snapshot;
}

bool
Snapshot::bufferHasSameType(const u8* buffer, size_t length)
{
    return Snapshot::isSnapshot(buffer, length);
}

bool
Snapshot::fileHasSameType(const char *path)
{
    return Snapshot::isSnapshotFile(path, V_MAJOR, V_MINOR, V_SUBMINOR);
}
