// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _VASTD_INC
#define _VASTD_INC

// General Includes
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <time.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sched.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>

#include "va_config.h"
#include "va_types.h"
#include "va_constants.h"

#if 0
// Two bit binary value
typedef uint8_t uint2_t;
inline bool is_uint2_t(uint2_t value) { return value < 4; }

//! @brief    Three bit binary value
typedef uint8_t uint3_t;

//! @brief    Integrity check
inline bool is_uint3_t(uint2_t value) { return value < 8; }

//! @brief    Four bit binary value
typedef uint8_t uint4_t;

//! @brief    Integrity check
inline bool is_uint4_t(uint4_t value) { return value < 16; }

//! @brief    Five bit binary value
typedef uint8_t uint5_t;

//! @brief    Integrity check
inline bool is_uint5_t(uint5_t value) { return value < 32; }
#endif

//
//! @functiongroup Converting units
//

// Macros for converting bytes to kilo bytes or mega bytes and vice versa
/*
#define B2KB(x) ((x) >> 10)
#define B2MB(x) ((x) >> 20)
#define KB2B(x) ((x) << 10)
#define MB2B(x) ((x) << 20)
*/

// Macros for converting kilo bytes and mega bytes to bytes
#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)

//
//! @functiongroup Handling low level data objects
//

//! @brief    Returns the high byte of a uint16_t value.
#define HI_BYTE(x) (uint8_t)((x) >> 8)

//! @brief    Returns the low byte of a uint16_t value.
#define LO_BYTE(x) (uint8_t)((x) & 0xFF)

//! @brief    Specifies a larger integer in little endian byte format
#define LO_HI(x,y) (uint16_t)((y) << 8 | (x))
#define LO_LO_HI(x,y,z) (uint32_t)((z) << 16 | (y) << 8 | (x))
#define LO_LO_HI_HI(x,y,z,w) (uint32_t)((w) << 24 | (z) << 16 | (y) << 8 | (x))

//! @brief    Specifies a larger integer in big endian byte format
#define HI_LO(x,y) (uint16_t)((x) << 8 | (y))
#define HI_HI_LO(x,y,z) (uint32_t)((x) << 16 | (y) << 8 | (z))
#define HI_HI_LO_LO(x,y,z,w) (uint32_t)((x) << 24 | (y) << 16 | (z) << 8 | (w))

//! @brief    Returns a certain byte of a larger integer
#define BYTE0(x) LO_BYTE(x)
#define BYTE1(x) LO_BYTE((x) >> 8)
#define BYTE2(x) LO_BYTE((x) >> 16)
#define BYTE3(x) LO_BYTE((x) >> 24)

//! @brief    Returns a non-zero value if the n-th bit is set in x.
#define GET_BIT(x,nr) ((x) & (1 << (nr)))

//! @brief    Sets a single bit.
#define SET_BIT(x,nr) ((x) |= (1 << (nr)))

//! @brief    Clears a single bit.
#define CLR_BIT(x,nr) ((x) &= ~(1 << (nr)))

//! @brief    Toggles a single bit.
#define TOGGLE_BIT(x,nr) ((x) ^= (1 << (nr)))

//! @brief    Sets a single bit to 0 (value == 0) or 1 (value != 0)
#define WRITE_BIT(x,nr,value) ((value) ? SET_BIT(x, nr) : CLR_BIT(x, nr))

//! @brief    Copies a single bit from x to y.
#define COPY_BIT(x,y,nr) ((y) = ((y) & ~(1 << (nr)) | ((x) & (1 << (nr)))))

//! @brief    Returns true if value is rising when switching from x to y
#define RISING_EDGE(x,y) (!(x) && (y))

//! @brief    Returns true if bit n is rising when switching from x to y
#define RISING_EDGE_BIT(x,y,n) (!((x) & (1 << (n))) && ((y) & (1 << (n))))

//! @brief    Returns true if value is falling when switching from x to y
#define FALLING_EDGE(x,y) ((x) && !(y))

//! @brief    Returns true if bit n is falling when switching from x to y
#define FALLING_EDGE_BIT(x,y,n) (((x) & (1 << (n))) && !((y) & (1 << (n))))


//
// Handling buffers
//

// Writes a byte value into a buffer.
inline void write8(uint8_t **ptr, uint8_t value) { *((*ptr)++) = value; }

// Writes a word value into a buffer in big endian format.
inline void write16(uint8_t **ptr, uint16_t value) {
    write8(ptr, (uint8_t)(value >> 8)); write8(ptr, (uint8_t)value); }

// Writes a double byte value into a buffer in big endian format.
inline void write32(uint8_t **ptr, uint32_t value) {
    write16(ptr, (uint16_t)(value >> 16)); write16(ptr, (uint16_t)value); }

// Writes a quad word value into a buffer in big endian format.
inline void write64(uint8_t **ptr, uint64_t value) {
    write32(ptr, (uint32_t)(value >> 32)); write32(ptr, (uint32_t)value); }

// Writes a memory block into a buffer in big endian format.
inline void writeBlock(uint8_t **ptr, uint8_t *values, size_t length) {
    memcpy(*ptr, values, length); *ptr += length; }

// Writes a word memory block into a buffer in big endian format.
inline void writeBlock16(uint8_t **ptr, uint16_t *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(uint16_t); i++) write16(ptr, values[i]); }

// Writes a double word memory block into a buffer in big endian format.
inline void writeBlock32(uint8_t **ptr, uint32_t *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(uint32_t); i++) write32(ptr, values[i]); }

// Writes a quad word memory block into a buffer in big endian format.
inline void writeBlock64(uint8_t **ptr, uint64_t *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(uint64_t); i++) write64(ptr, values[i]); }


// Reads a byte value from a buffer.
inline uint8_t read8(uint8_t **ptr) { return (uint8_t)(*((*ptr)++)); }

// Reads a word value from a buffer in big endian format.
inline uint16_t read16(uint8_t **ptr) {
    return ((uint16_t)read8(ptr) << 8) | (uint16_t)read8(ptr); }

// Reads a double word value from a buffer in big endian format.
inline uint32_t read32(uint8_t **ptr) {
    return ((uint32_t)read16(ptr) << 16) | (uint32_t)read16(ptr); }

// Reads a quad word value from a buffer in big endian format.
inline uint64_t read64(uint8_t **ptr) {
    return ((uint64_t)read32(ptr) << 32) | (uint64_t)read32(ptr); }

// Reads a memory block from a buffer.
inline void readBlock(uint8_t **ptr, uint8_t *values, size_t length) {
    memcpy(values, *ptr, length); *ptr += length; }

// Reads a word block from a buffer in big endian format.
inline void readBlock16(uint8_t **ptr, uint16_t *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(uint16_t); i++) values[i] = read16(ptr); }

// Reads a double word block from a buffer in big endian format.
inline void readBlock32(uint8_t **ptr, uint32_t *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(uint32_t); i++) values[i] = read32(ptr); }

// Reads a quad word block from a buffer in big endian format.
inline void readBlock64(uint8_t **ptr, uint64_t *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(uint64_t); i++) values[i] = read64(ptr); }


//
// Converting low level data objects
//

#if 0
/*! @brief    Translates a PETSCII string to a unichar array.
 *  @details  This functions creates unicode characters compatible with the
 *            C64ProMono font. The target font supports four different mapping
 *            tables starting at different base addresses:
 *
 *            0xE000 : Unshifted (only upper case characters)
 *            0xE100 : Shifted   (upper and lower case characters)
 *            0xE200 : Unshifted, reversed
 *            0xE300 : Shifted, reversed
 *
 *  @note     A maximum of max characters are translated.
 *            The unicode array will always be terminated by a NULL character.
 */
void translateToUnicode(const char *petscii, uint16_t *unichars,
                        uint16_t base, size_t max);

//! @brief    Returns the number of characters in a null terminated unichar array
size_t strlen16(const uint16_t *unichars);

/*! @brief    Converts a PETSCII character to a printable character.
 *  @details  Replaces all unprintable characters by subst.
 */
uint8_t petscii2printable(uint8_t c, uint8_t subst);

/*! @brief    Converts an ASCII character to a PETSCII character.
 *  @details  This function translates into the unshifted PET character set.
 *            I.e., lower case characters are converted to uppercase characters.
 *  @result   Returns ' ' for ASCII characters with no PETSCII representation.
 */
uint8_t ascii2pet(uint8_t asciichar);

//! @brief    Converts an ASCII string into a PETSCII string.
/*! @details  Applies function ascii2pet to all characters of a string.
 */
void ascii2petStr(char *str);

#endif

//! @brief    Writes an uint8_t value into a string in decimal format
void sprint8d(char *s, uint8_t value);

//! @brief    Writes an uint8_t value into a string in hexadecimal format
void sprint8x(char *s, uint8_t value);

//! @brief    Writes an uint8_t value into a string in binary format
void sprint8b(char *s, uint8_t value);

//! @brief    Writes an uint16_t value into a string in decimal format
void sprint16d(char *s, uint16_t value);

//! @brief    Writes an uint16_t value into a string in hexadecimal format
void sprint16x(char *s, uint16_t value);

//! @brief    Writes an uint16_t value into a string in binary format
void sprint16b(char *s, uint16_t value);


//
// Handling files
//

/* Extracts the filename from a path
 * Returns a newly created string. You need to delete it manually.
 */
char *extractFilename(const char *path);

/* Extracts file suffix from a path
 * Returns a newly created string. You need to delete it manually.
 */
char *extractSuffix(const char *path);

/* Extracts filename from a path without its suffix
 * Returns a newly created string. You need to delete it manually.
 */
char *extractFilenameWithoutSuffix(const char *path);

/* Check file suffix
 * The function is used for determining the type of a file.
 */
bool checkFileSuffix(const char *filename, const char *suffix);

// Returns the size of a file in bytes
long getSizeOfFile(const char *filename);

// Checks the size of a file
bool checkFileSize(const char *filename, long size);
bool checkFileSizeRange(const char *filename, long min, long max);

/* Checks the header signature (magic bytes) of a file.
 * This function is used for determining the type of a file.
 *   - path      File name, must not be Null
 *   - header    Expected byte sequence.
 *   - length    Length of the expected byte sequence in bytes
 */
bool matchingFileHeader(const char *path, const uint8_t *header, size_t length);

/* Checks the header signature (magic bytes) of a buffer.
 * This function is used for determining the type of a file.
 *   - buffer    Pointer to buffer, must not be NULL
 *   - header    Expected byte sequence
 *   - length    Length of the expected byte sequence in bytes
 */
bool matchingBufferHeader(const uint8_t *buffer, const uint8_t *header, size_t length);


//
//! @functiongroup Managing time
//


/*! @brief    Application launch time in seconds
 *  @details  The value is read by function msec for computing the elapsed
 *            number of microseconds.
 */
// extern long tv_base;

//! @brief    Return the number of elapsed microseconds since program launch.
// uint64_t usec();

//! @brief    Reads the real-time clock (1/10th seconds).
uint8_t localTimeSecFrac();

//! @brief    Reads the real-time clock (seconds).
uint8_t localTimeSec();

//! @brief    Reads the real-time clock (minutes).
uint8_t localTimeMin();

//! @brief    Reads the real-time clock (hours).
uint8_t localTimeHour();

//! @brief    Put the current thread to sleep for a certain amount of time.
void sleepMicrosec(unsigned usec);

/*! @brief    Sleeps until kernel timer reaches kernelTargetTime
 *  @param    kernelEarlyWakeup To increase timing precision, the function
 *            wakes up the thread earlier by this amount and waits actively in
 *            a delay loop until the deadline is reached.
 *  @return   Overshoot time (jitter), measured in kernel time. Smaller values
 *            are better, 0 is best.
 */
int64_t sleepUntil(uint64_t kernelTargetTime, uint64_t kernelEarlyWakeup);


//
// Computing fingerprints
//

//! @brief    Computes a fingeprint based on the FNV-1a hash algorithm
uint64_t fnv_1a(uint8_t *addr, size_t size);


//
// Debugging
//

// Returns true if we're running a release build.
bool releaseBuild();
    
    
#endif
