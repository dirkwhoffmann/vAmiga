// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _VAFILE_INC
#define _VAFILE_INC

#include "VAObject.h"

/* Base class of all file readable types.
 * Provides the basic functionality for reading and writing files.
 */
class VAFile : public VAObject {
    
protected:
    
    // Physical location of this file on disk
    char *path = NULL;
    
    // The raw data of this file
    uint8_t *data = NULL;
    
    // The size of this file in bytes
    size_t size = 0;
    
    /* File pointer
     * An offset into the data array with -1 indicating EOF
     */
    long fp = -1;
    
    /* End of file position
     * This value equals the last valid offset plus 1
     */
    long eof = -1;
    
    
protected:
    
    /* Checks the header signature of a buffer.
     *  buffer    Pointer to buffer, must not be NULL
     *  length    Length of the buffer
     *  header    Expected byte sequence, terminated by EOF
     *  Returns   true iff magic bytes match.
     */
    static bool checkBufferHeader(const uint8_t *buffer, size_t length,
                                  const uint8_t *header);
    
    
    //
    // Creating and destructing objects
    //
    
public:
    
    VAFile();
    virtual ~VAFile();
    
    // Frees the memory allocated by this object.
    virtual void dealloc();
    
    
    //
    // Accessing file attributes
    //
    
    // Returns the type of this file.
    virtual VAFileType type() { return FILETYPE_UKNOWN; }
    
    // Returns a string representation of the file type, e.g., "ADF".
    virtual const char *typeAsString() { return ""; }
    
    // Returns the physical name of this file.
    const char *getPath() { return path ? path : ""; }
    
    // Sets the physical name of this file.
    void setPath(const char *path);
    
    
    //
    // Reading data from the file
    //
    
    //  Returns the number of bytes in this file.
    virtual size_t getSize() { return size; }
    
    // Moves the file pointer to the specified offset.
    virtual void seek(long offset) { fp = (offset < size) ? offset : -1; }
    
    /*  Reads a byte.
     *  Returns EOF (-1) if the end of file has been reached.
     */
    virtual int read();
    
    /* Reads multiple bytes in form of a hex dump string.
     * Number of bytes ranging from 1 to 85.
     */
    // const char *readHex(size_t num = 1);
    
    //! Copies the whole file data into a buffer.
    virtual void flash(uint8_t *buffer, size_t offset = 0);
    
    
    //
    // Serializing
    //
    
    // Returns the required buffer size for this file
    size_t sizeOnDisk() { return writeToBuffer(NULL); }
    
    /* Returns true iff this file has the same type as the file stored in the
     * specified file.
     */
    virtual bool hasSameType(const char *filename) { return false; }
    
    /* Reads the file contents from a memory buffer.
     *   - buffer   The address of a binary representation in memory.
     *   - length   The size of the binary representation.
     */
    virtual bool readFromBuffer(const uint8_t *buffer, size_t length);
    
    /* Reads the file contents from a file.
     * This function requires no custom implementation. It first reads in the
     * file contents in memory and invokes readFromBuffer afterwards.
     */
    bool readFromFile(const char *filename);
    
    /* Writes the file contents into a memory buffer.
     * If a NULL pointer is passed in, a test run is performed. Test runs can
     * be performed to determine the size of the file on disk.
     */
    virtual size_t writeToBuffer(uint8_t *buffer);
    
    /* Writes the file contents to a file.
     * This function requires no custom implementation. It invokes writeToBuffer
     * first and writes the data to disk afterwards.
     *   - filename   The name of a file to be written.
     */
    bool writeToFile(const char *filename);
};

#endif
