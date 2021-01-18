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
#include "Folder.h"
#include "HDFFile.h"
#include "RomFile.h"
#include "ExtendedRomFile.h"


AmigaFile::AmigaFile(usize capacity)
{
    data = new u8[capacity]();
    size = capacity;
}

AmigaFile::~AmigaFile()
{
    if (data) delete[] data;
}

bool
AmigaFile::alloc(size_t capacity)
{
    if ((data = new u8[capacity]()) == nullptr) return false;
    size = capacity;
    
    return true;
}

/*
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
}
*/

void
AmigaFile::flash(u8 *buffer, size_t offset)
{
    assert(buffer != nullptr);
    memcpy(buffer + offset, data, size);
}

usize
AmigaFile::readFromStream(std::istream &stream)
{
    // Get stream size
    auto fsize = stream.tellg();
    stream.seekg(0, std::ios::end);
    fsize = stream.tellg() - fsize;
    stream.seekg(0, std::ios::beg);

    // Allocate memory
    assert(data == nullptr);
    data = new u8[fsize]();
    size = fsize;

    // Fix known inconsistencies
    stream.read((char *)data, size);
    
    // Repair the file (if applicable)
    repair();
    
    return size;
}

usize
AmigaFile::readFromFile(const char *path)
{
    assert(path);
        
    std::ifstream stream(path);

    if (!stream.is_open()) {
        throw VAError(ERROR_FILE_CANT_READ);
    }
    
    usize result = readFromStream(stream);
    assert(result == size);
    
    this->path = string(path);
    return size;
}

usize
AmigaFile::readFromBuffer(const u8 *buf, usize len)
{
    assert(buf);

    std::istringstream stream(std::string((const char *)buf, len));
    
    usize result = readFromStream(stream);
    assert(result == size);
    return size;
}

usize
AmigaFile::writeToStream(std::ostream &stream)
{
    stream.write((char *)data, size);
    return size;
}

usize
AmigaFile::writeToStream(std::ostream &stream, ErrorCode *err)
{
    *err = ERROR_OK;
    try { return writeToStream(stream); }
    catch (VAError &exception) { *err = exception.errorCode; }
    return 0;
}

usize
AmigaFile::writeToFile(const char *path)
{
    assert(path);
        
    std::ofstream stream(path);

    if (!stream.is_open()) {
        throw VAError(ERROR_FILE_CANT_WRITE);
    }
    
    usize result = writeToStream(stream);
    assert(result == size);
    
    return size;
}

usize
AmigaFile::writeToFile(const char *path, ErrorCode *err)
{
    *err = ERROR_OK;
    try { return writeToFile(path); }
    catch (VAError &exception) { *err = exception.errorCode; }
    return 0;
}

usize
AmigaFile::writeToBuffer(u8 *buf)
{
    assert(buf);

    std::ostringstream stream;
    usize len = writeToStream(stream);
    stream.write((char *)buf, len);
    
    return len;
}

usize
AmigaFile::writeToBuffer(u8 *buf, ErrorCode *err)
{
    *err = ERROR_OK;
    try { return writeToBuffer(buf); }
    catch (VAError &exception) { *err = exception.errorCode; }
    return 0;
}



/*

bool
AmigaFile::readFromBuffer(const u8 *buffer, size_t length, ErrorCode *error)
{
    assert (buffer != nullptr);
    
    // Check file type
    if (!matchingBuffer(buffer, length)) {
        if (error) *error = ERROR_FILE_TYPE_MISMATCH;
        return false;
    }
    
    // Allocate memory
    if (!alloc(length)) {
        if (error) *error = ERROR_OUT_OF_MEMORY;
        return false;
    }
    
    // Read from buffer
    memcpy(data, buffer, length);
 
    if (error) *error = ERROR_OK;
    return true;
}

bool
AmigaFile::readFromFile(const char *filename, ErrorCode *error)
{
    assert (filename != nullptr);
    
    bool success;
    FILE *file = nullptr;
    struct stat fileProperties;
    
    // Get properties
    if (stat(filename, &fileProperties) != 0) {
        if (error) *error = ERROR_FILE_NOT_FOUND;
        return false;
    }

    // Check type
    if (!matchingFile(filename)) {
        if (error) *error = ERROR_FILE_TYPE_MISMATCH;
        return false;
    }
        
    // Open
    if (!(file = fopen(filename, "r"))) {
        if (error) *error = ERROR_FILE_CANT_READ;
        return false;
    }
    
    // Read
    setPath(filename);
    success = readFromFile(file, error);
    
    fclose(file);
    return success;
}

bool
AmigaFile::readFromFile(FILE *file, ErrorCode *error)
{
    assert (file != nullptr);
    
    u8 *buffer = nullptr;

    // Get size
    fseek(file, 0, SEEK_END);
    size_t size = (size_t)ftell(file);
    rewind(file);
    
    // Allocate memory
    if (!(buffer = new u8[size])) {
        if (error) *error = ERROR_OUT_OF_MEMORY;
        return false;
    }

    // Read from file
    int c;
    for (unsigned i = 0; i < size; i++) {
        if ((c = fgetc(file)) == EOF) break;
        buffer[i] = (u8)c;
    }
    
    // Read from buffer
    dealloc();
    if (!readFromBuffer(buffer, size, error)) {
        delete[] buffer;
        return false;
    }
    
    delete[] buffer;
    if (error) *error = ERROR_OK;
    return true;
}

size_t
AmigaFile::writeToBuffer(u8 *buffer) const
{
    assert(data);
    
    if (buffer) {
        memcpy(buffer, data, size);
    }
    return size;
}

bool
AmigaFile::writeToFile(const char *filename, ErrorCode *error) const
{
    assert (filename);

    bool success = false;
    u8 *data = nullptr;
    FILE *file;
    
    // Determine the size of the file in bytes
    size_t filesize = writeToBuffer(nullptr);
    
    // Open file
    if (!(file = fopen(filename, "w"))) {
        if (error) *error = ERROR_FILE_CANT_WRITE;
        goto exit;
    }
    // Allocate a buffer
    if (!(data = new u8[filesize])) {
        if (error) *error = ERROR_OUT_OF_MEMORY;
        goto exit;
    }
    // Write contents to the created buffer
    (void)writeToBuffer(data);
    
    // Write the buffer to a file
    for (unsigned i = 0; i < filesize; i++) fputc(data[i], file);
    *error = ERROR_OK;
    success = true;
    
exit:
    
    if (file)
        fclose(file);
    if (data)
        delete[] data;
    
    return success;
}
*/
 
//
// Instantiate template functions
//

template Snapshot* AmigaFile::make <Snapshot> (const u8 *, size_t, ErrorCode *);
template ADFFile* AmigaFile::make <ADFFile> (const u8 *, size_t, ErrorCode *);
template EXTFile* AmigaFile::make <EXTFile> (const u8 *, size_t, ErrorCode *);
template IMGFile* AmigaFile::make <IMGFile> (const u8 *, size_t, ErrorCode *);
template DMSFile* AmigaFile::make <DMSFile> (const u8 *, size_t, ErrorCode *);
template EXEFile* AmigaFile::make <EXEFile> (const u8 *, size_t, ErrorCode *);
// template Folder* AmigaFile::make <Folder> (const u8 *, size_t, ErrorCode *);
template HDFFile* AmigaFile::make <HDFFile> (const u8 *, size_t, ErrorCode *);
template RomFile* AmigaFile::make <RomFile> (const u8 *, size_t, ErrorCode *);
template ExtendedRomFile* AmigaFile::make <ExtendedRomFile> (const u8 *, size_t, ErrorCode *);

template Snapshot* AmigaFile::make <Snapshot> (const char *, ErrorCode *);
template ADFFile* AmigaFile::make <ADFFile> (const char *, ErrorCode *);
template EXTFile* AmigaFile::make <EXTFile> (const char *, ErrorCode *);
template IMGFile* AmigaFile::make <IMGFile> (const char *, ErrorCode *);
template DMSFile* AmigaFile::make <DMSFile> (const char *, ErrorCode *);
template EXEFile* AmigaFile::make <EXEFile> (const char *, ErrorCode *);
// template Folder* AmigaFile::make <Folder> (const char *, ErrorCode *);
template HDFFile* AmigaFile::make <HDFFile> (const char *, ErrorCode *);
template RomFile* AmigaFile::make <RomFile> (const char *, ErrorCode *);
template ExtendedRomFile* AmigaFile::make <ExtendedRomFile> (const char *, ErrorCode *);
template ADFFile* AmigaFile::make <ADFFile> (FILE *, ErrorCode *);
