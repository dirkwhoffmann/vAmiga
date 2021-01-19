// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaObject.h"
#include "FileTypes.h"

// Base class for all supported file types
class AmigaFile : public AmigaObject {
    
public:
    
    // Physical location of this file
    string path = "";
    
    // The raw data of this file
    u8 *data = nullptr;
    
    // The size of this file in bytes
    usize size = 0;
    
    
    //
    // Creating
    //
    
public:
    
    template <class T> static T *make(const string &path, std::istream &stream) throws
    {
        if (!T::isCompatibleStream(stream)) throw VAError(ERROR_FILE_TYPE_MISMATCH);
        
        T *obj = new T();
        obj->path = path;
        
        try { obj->readFromStream(stream); } catch (VAError &err) {
            delete obj;
            throw err;
        }
        return obj;
    }

    template <class T> static T *make(const string &path, std::istream &stream, ErrorCode *err)
    {
        *err = ERROR_OK;
        try { return make <T> (stream); }
        catch (VAError &exception) { *err = exception.errorCode; }
        return nullptr;
    }
    
    template <class T> static T *make(const u8 *buf, usize len) throws
    {
        std::stringstream stream;
        stream.write((const char *)buf, len);
        return make <T> ("", stream);
    }
    
    template <class T> static T *make(const u8 *buf, usize len, ErrorCode *err)
    {
        *err = ERROR_OK;
        try { return make <T> (buf, len); }
        catch (VAError &exception) { *err = exception.errorCode; }
        return nullptr;
    }
    
    template <class T> static T *make(const char *path) throws
    {
        std::ifstream stream(path);
        if (!stream.is_open()) throw VAError(ERROR_FILE_NOT_FOUND);

        T *file = make <T> (string(path), stream);
        return file;
    }

    template <class T> static T *make(const char *path, ErrorCode *err)
    {
        *err = ERROR_OK;
        try { return make <T> (path); }
        catch (VAError &exception) { *err = exception.errorCode; }
        return nullptr;
    }

    template <class T> static T *make(FILE *file) throws
    {
        std::stringstream stream;
        int c; while ((c = fgetc(file)) != EOF) { stream.put(c); }
        return make <T> ("", stream);
    }
    
    template <class T> static T *make(FILE *file, ErrorCode *err)
    {
        *err = ERROR_OK;
        try { return make <T> (file); }
        catch (VAError &exception) { *err = exception.errorCode; }
        return nullptr;
    }
    
    
    //
    // Initializing
    //
    
public:

    AmigaFile() { };
    AmigaFile(usize capacity);
    virtual ~AmigaFile();
    
    // Allocates memory for storing the object data
    virtual bool alloc(usize capacity);
    
    // Frees the allocated memory
    // virtual void dealloc();
    
    
    //
    // Accessing file attributes
    //
    
    // Returns the type of this file
    virtual FileType type() const { return FILETYPE_UKNOWN; }
            
    // Returns a fingerprint (hash value) for this file
    virtual u64 fnv() const { return fnv_1a_64(data, size); }
        
    
    //
    // Flashing data
    //
            
    // Copies the file contents into a buffer starting at the provided offset
    virtual void flash(u8 *buf, usize offset = 0);
    
    
    //
    // Serializing
    //
protected:
    
    virtual usize readFromStream(std::istream &stream) throws;
    usize readFromFile(const char *path) throws;
    usize readFromBuffer(const u8 *buf, usize len) throws;

public:
    
    virtual usize writeToStream(std::ostream &stream) throws;
    usize writeToStream(std::ostream &stream, ErrorCode *err);

    usize writeToFile(const char *path) throws;
    usize writeToFile(const char *path, ErrorCode *err);
    
    usize writeToBuffer(u8 *buf) throws;
    usize writeToBuffer(u8 *buf, ErrorCode *err);
    
    
    
    // Returns the required buffer size for this file
    // [[deprecated]] usize sizeOnDisk() const { return writeToBuffer(nullptr); }
    
    /* Returns true iff this specified buffer is compatible with this object.
     * This function is used in readFromBuffer().
     */
    [[deprecated]] virtual bool matchingBuffer(const u8 *buf, usize len) { return false; }

    /* Returns true iff this specified file is compatible with this object.
     * This function is used in readFromFile().
     */
    [[deprecated]] virtual bool matchingFile(const char *path) { return false; }


    //
    // Repairing
    //
    
public:
    
    /* This function is called in the default implementation of readFromStream.
     * It can be overwritten to fix known inconsistencies in certain media
     * files.
     */
    virtual void repair() { };
};
