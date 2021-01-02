// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_FILE_H
#define _AMIGA_FILE_H

#include "AmigaObject.h"

// Base class for all supported file types
class AmigaFile : public AmigaObject {
    
protected:
    
    // Physical location of this file on disk (if known)
    char *path = nullptr;
    
    // The raw data of this file
    u8 *data = nullptr;
    
    // The size of this file in bytes
    usize size = 0;
    
    
    //
    // Creating
    //
    
public:
    
    template <class T> static T *make(const u8 *buf, usize len, FileError *err = nullptr);
    template <class T> static T *make(const char *path, FileError *err = nullptr);
    template <class T> static T *make(FILE *file, FileError *err = nullptr);

    
    //
    // Initializing
    //
    
public:

    AmigaFile();
    virtual ~AmigaFile();
    
    // Allocates memory for storing the object data
    virtual bool alloc(usize capacity);
    
    // Frees the allocated memory
    virtual void dealloc();
    
    
    //
    // Accessing file attributes
    //
    
    // Returns the type of this file
    virtual FileType fileType() { return FILETYPE_UKNOWN; }
        
    // Returns the physical name of this file
    const char *getPath() { return path ? path : ""; }
    
    // Sets the physical name of this file
    void setPath(const char *path);
    
    // Returns a fingerprint (hash value) for this file
    virtual u64 fnv() { return fnv_1a_64(data, size); }
    
    
    //
    // Reading data from the file
    //
    
    // Returns a pointer to the raw data of this file
    virtual u8 *getData() { return data; }

    // Returns the number of bytes in this file
    virtual usize getSize() { return size; }
            
    // Copies the whole file data into a buffer
    virtual void flash(u8 *buf, usize offset = 0);
    
    
    //
    // Serializing
    //
    
    // Returns the required buffer size for this file
    usize sizeOnDisk() { return writeToBuffer(nullptr); }

    /* Returns true iff this specified buffer is compatible with this object.
     * This function is used in readFromBuffer().
     */
    virtual bool matchingBuffer(const u8 *buf, usize len) { return false; }

    /* Returns true iff this specified file is compatible with this object.
     * This function is used in readFromFile().
     */
    virtual bool matchingFile(const char *path) { return false; }
    
    /* Deserializes this object from a memory buffer. This function uses
     * matchingBuffer() to verify that the buffer contains a compatible
     * binary representation.
     */
    virtual bool readFromBuffer(const u8 *buf, usize len, FileError *error = nullptr);

    /* Deserializes this object from a file. This function uses
     * matchingFile() to verify that the file contains a compatible binary
     * representation. This function requires no custom implementation. It
     * first reads in the file contents in memory and invokes readFromBuffer
     * afterwards.
     */
    virtual bool readFromFile(const char *filename, FileError *err = nullptr);

    /* Deserializes this object from a file that is already open.
     */
    virtual bool readFromFile(FILE *file, FileError *err = nullptr);

    /* Writes the file contents into a memory buffer. If nullptr is
     * passed in, a test run is performed. Test runs can be performed to
     * determine the size of the file on disk.
     */
    virtual usize writeToBuffer(u8 *buf);
    
    /* Writes the file contents to a file. This function requires no custom
     * implementation. It invokes writeToBuffer first and writes the data to
     * disk afterwards.
     */
    bool writeToFile(const char *path, FileError *err = nullptr);
};

#endif
