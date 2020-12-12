// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "AmigaFile.h"
#include "Snapshot.h"
#include "ADFFile.h"
#include "EXTFile.h"
#include "IMGFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "DIRFile.h"
#include "HDFFile.h"
#include "RomFile.h"
#include "EncryptedRomFile.h"
#include "ExtendedRomFile.h"

template <class T> T *
AmigaFile::make(const u8 *buffer, size_t length, FileError *err)
{
    T *obj = new T();
    
    if (!obj->readFromBuffer(buffer, length)) {
        delete obj;
        return nullptr;
    }
        
    return obj;
}

template <class T> T *
AmigaFile::make(const char *path, FileError *err)
{
    T *obj = new T();
    
    if (!obj->readFromFile(path)) {
        delete obj;
        return nullptr;
    }
    
    return obj;
}

template <class T> T *
AmigaFile::make(FILE *file, FileError *err)
{
    T *obj = new T();
    
    if (!obj->readFromFile(file)) {
        delete obj;
        return nullptr;
    }
    
    return obj;
}

AmigaFile::AmigaFile()
{
}

AmigaFile::~AmigaFile()
{
    dealloc();
    
    if (path)
        free(path);
}

bool
AmigaFile::alloc(size_t capacity)
{
    dealloc();
    
    if ((data = new u8[capacity]()) == nullptr)
        return false;
    
    size = eof = capacity;
    fp = 0;
    
    return true;
}

void
AmigaFile::dealloc()
{
    if (data == nullptr) {
        assert(size == 0);
        return;
    }
    
    delete[] data;
    data = nullptr;
    
    size = 0;
    fp = -1;
    eof = -1;
}

void
AmigaFile::setPath(const char *str)
{
    assert(str != nullptr);
    
    // Set path
    if (path) free(path);
    path = strdup(str);
}

void
AmigaFile::seek(long offset)
{
    eof = size;
    fp = (offset < eof) ? offset : -1;
}

int
AmigaFile::read()
{
    int result;
    
    assert(eof <= (long)size);
    
    if (fp < 0)
        return -1;
    
    // Get byte
    result = data[fp++];
    
    // Check for end of file
    if (fp == eof)
        fp = -1;
    
    return result;
}

void
AmigaFile::flash(u8 *buffer, size_t offset)
{
    int byte;
    assert(buffer != nullptr);
    
    seek(0);
    
    while ((byte = read()) != EOF) {
        buffer[offset++] = (u8)byte;
    }
}

bool
AmigaFile::readFromBuffer(const u8 *buffer, size_t length)
{
    FileError error;
    return readFromBuffer(buffer, length, &error);
}

bool
AmigaFile::readFromBuffer(const u8 *buffer, size_t length, FileError *error)
{
    assert (buffer != nullptr);
    
    // Check file type
    if (!bufferHasSameType(buffer, length)) {
        *error = ERR_INVALID_FILE_TYPE;
        return false;
    }
    
    // Allocate memory
    if (!alloc(length)) {
        *error = ERR_OUT_OF_MEMORY;
        return false;
    }
    
    // Read from buffer
    memcpy(data, buffer, length);
 
    *error = ERR_FILE_OK;
    return true;
}

bool
AmigaFile::readFromFile(const char *filename)
{
    FileError error;
    return readFromFile(filename, &error);
}

bool
AmigaFile::readFromFile(const char *filename, FileError *error)
{
    assert (filename != NULL);
    
    bool success;
    FILE *file = NULL;
    struct stat fileProperties;
    
    // Get file properties
    if (stat(filename, &fileProperties) != 0) {
        *error = ERR_NO_SUCH_FILE;
        return false;
    }

    // Check file type
    if (!fileHasSameType(filename)) {
        *error = ERR_INVALID_FILE_TYPE;
        return false;
    }
        
    // Open file
    if (!(file = fopen(filename, "r"))) {
        *error = ERR_CANT_OPEN_FOR_READ;
        return false;
    }
    
    // Read file
    if ((success = readFromFile(file, error))) {
        setPath(filename);
    }
    
    fclose(file);
    return success;
}

bool
AmigaFile::readFromFile(FILE *file)
{
    FileError error;
    return readFromFile(file, &error);
}

bool
AmigaFile::readFromFile(FILE *file, FileError *error)
{
    assert (file != nullptr);
    
    u8 *buffer = nullptr;

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t size = (size_t)ftell(file);
    rewind(file);
    
    // Allocate memory
    if (!(buffer = new u8[size])) {
        *error = ERR_OUT_OF_MEMORY;
        return false;
    }

    // Read from file
    int c;
    for (unsigned i = 0; i < size; i++) {
        if ((c = fgetc(file)) == EOF) break;
        buffer[i] = (u8)c;
    }

    // Check type
    if (!bufferHasSameType(buffer, size)) {
        *error = ERR_INVALID_FILE_TYPE;
        delete[] buffer;
        return false;
    }
    
    // Read from buffer
    dealloc();
    if (!readFromBuffer(buffer, size)) {
        *error = ERR_INVALID_FILE_TYPE;
        delete[] buffer;
        return false;
    }
    
    delete[] buffer;
    return true;
}

size_t
AmigaFile::writeToBuffer(u8 *buffer)
{
    assert(data != nullptr);
    
    if (buffer) {
        memcpy(buffer, data, size);
    }
    return size;
}

bool
AmigaFile::writeToFile(const char *filename)
{
    assert (filename != nullptr);

    bool success = false;
    u8 *data = nullptr;
    FILE *file;
    size_t filesize;
    
    // Determine the size of the file in bytes
    if (!(filesize = writeToBuffer(nullptr))) return false;
    
    // Open file
    if (!(file = fopen(filename, "w"))) goto exit;
    
    // Allocate a buffer
    if (!(data = new u8[filesize])) goto exit;
    
    // Write contents to the created buffer
    if (!writeToBuffer(data)) goto exit;
    
    // Write the buffer to a file
    for (unsigned i = 0; i < filesize; i++) fputc(data[i], file);
    success = true;
    
exit:
    
    if (file)
        fclose(file);
    if (data)
        delete[] data;
    
    return success;
}

//
// Instantiate template functions
//

template Snapshot* AmigaFile::make <Snapshot> (const u8 *, size_t, FileError *);
template ADFFile* AmigaFile::make <ADFFile> (const u8 *, size_t, FileError *);
template EXTFile* AmigaFile::make <EXTFile> (const u8 *, size_t, FileError *);
template IMGFile* AmigaFile::make <IMGFile> (const u8 *, size_t, FileError *);
template DMSFile* AmigaFile::make <DMSFile> (const u8 *, size_t, FileError *);
template EXEFile* AmigaFile::make <EXEFile> (const u8 *, size_t, FileError *);
template DIRFile* AmigaFile::make <DIRFile> (const u8 *, size_t, FileError *);
template HDFFile* AmigaFile::make <HDFFile> (const u8 *, size_t, FileError *);
template RomFile* AmigaFile::make <RomFile> (const u8 *, size_t, FileError *);
template ExtendedRomFile* AmigaFile::make <ExtendedRomFile> (const u8 *, size_t, FileError *);
template EncryptedRomFile* AmigaFile::make <EncryptedRomFile> (const u8 *, size_t, FileError *);

template Snapshot* AmigaFile::make <Snapshot> (const char *, FileError *);
template ADFFile* AmigaFile::make <ADFFile> (const char *, FileError *);
template EXTFile* AmigaFile::make <EXTFile> (const char *, FileError *);
template IMGFile* AmigaFile::make <IMGFile> (const char *, FileError *);
template DMSFile* AmigaFile::make <DMSFile> (const char *, FileError *);
template EXEFile* AmigaFile::make <EXEFile> (const char *, FileError *);
template DIRFile* AmigaFile::make <DIRFile> (const char *, FileError *);
template HDFFile* AmigaFile::make <HDFFile> (const char *, FileError *);
template RomFile* AmigaFile::make <RomFile> (const char *, FileError *);
template ExtendedRomFile* AmigaFile::make <ExtendedRomFile> (const char *, FileError *);
template EncryptedRomFile* AmigaFile::make <EncryptedRomFile> (const char *, FileError *);

template ADFFile* AmigaFile::make <ADFFile> (FILE *, FileError *);
