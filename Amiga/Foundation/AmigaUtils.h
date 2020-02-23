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

#include "AmigaConfig.h"
#include "AmigaTypes.h"
#include "AmigaConstants.h"


//
// Optimizing code
//

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)


//
// Converting units
//

// Macros for converting kilo bytes and mega bytes to bytes
#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)

// Macros for converting kilo Hertz and mega Hertz to Hertz
#define KHz(x) ((x) * 1000)
#define MHz(x) ((x) * 1000000)


//
// Accessing bits and bytes
//

// Returns the low byte or the high byte of a uint16_t value
#define LO_BYTE(x) (uint8_t)((x) & 0xFF)
#define HI_BYTE(x) (uint8_t)((x) >> 8)

// Returns the low word or the high word of a uint32_t value
#define LO_WORD(x) (uint16_t)((x) & 0xFFFF)
#define HI_WORD(x) (uint16_t)((x) >> 16)

// Specifies a larger integer in little endian byte format
#define LO_HI(x,y) (uint16_t)((y) << 8 | (x))
#define LO_LO_HI(x,y,z) (uint32_t)((z) << 16 | (y) << 8 | (x))
#define LO_LO_HI_HI(x,y,z,w) (uint32_t)((w) << 24 | (z) << 16 | (y) << 8 | (x))
#define LO_W_HI_W(x,y) (uint32_t)((y) << 16 | (x))

// Specifies a larger integer in big endian byte format
#define HI_LO(x,y) (uint16_t)((x) << 8 | (y))
#define HI_HI_LO(x,y,z) (uint32_t)((x) << 16 | (y) << 8 | (z))
#define HI_HI_LO_LO(x,y,z,w) (uint32_t)((x) << 24 | (y) << 16 | (z) << 8 | (w))
#define HI_W_LO_W(x,y) (uint32_t)((x) << 16 | (y))

// Returns a certain byte of a larger integer
#define BYTE0(x) LO_BYTE(x)
#define BYTE1(x) LO_BYTE((x) >> 8)
#define BYTE2(x) LO_BYTE((x) >> 16)
#define BYTE3(x) LO_BYTE((x) >> 24)

// Returns a non-zero value if the n-th bit is set in x
#define GET_BIT(x,nr) ((x) & (1 << (nr)))

// Sets, clears, or toggles single bits
#define SET_BIT(x,nr) ((x) |= (1 << (nr)))
#define CLR_BIT(x,nr) ((x) &= ~(1 << (nr)))
#define TOGGLE_BIT(x,nr) ((x) ^= (1 << (nr)))

// Replaces bits, bytes, and words
#define REPLACE_BIT(x,nr,v) ((v) ? SET_BIT(x, nr) : CLR_BIT(x, nr))
#define REPLACE_LO(x,y) (((x) & ~0x00FF) | y)
#define REPLACE_HI(x,y) (((x) & ~0xFF00) | ((y) << 8))
#define REPLACE_LO_WORD(x,y) (((x) & ~0xFFFF) | y)
#define REPLACE_HI_WORD(x,y) (((x) & ~0xFFFF0000) | ((y) << 16))

// Checks for a rising or a falling edge
#define RISING_EDGE(x,y) (!(x) && (y))
#define RISING_EDGE_BIT(x,y,n) (!((x) & (1 << (n))) && ((y) & (1 << (n))))
#define FALLING_EDGE(x,y) ((x) && !(y))
#define FALLING_EDGE_BIT(x,y,n) (((x) & (1 << (n))) && !((y) & (1 << (n))))

// Checks is a number is even or odd
#define IS_EVEN(x) (!IS_ODD(x))
#define IS_ODD(x) ((x) & 1)

// Rounds a number up or down to the next even or odd number
#define UP_TO_NEXT_EVEN(x) ((x) + ((x) & 1))
#define DOWN_TO_NEXT_EVEN(x) ((x) & (~1))
#define UP_TO_NEXT_ODD(x) ((x) | 1)
#define DOWN_TO_NEXT_ODD(x) ((x) - !((x) & 1))


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

/* Compares the file suffix with a given string
 * The function is used for determining the type of a file.
 */
bool checkFileSuffix(const char *filename, const char *suffix);

// Returns the size of a file in bytes
long getSizeOfFile(const char *filename);

// Checks the size of a file
bool checkFileSize(const char *filename, long size);
bool checkFileSizeRange(const char *filename, long min, long max);

// Checks the header signature (magic bytes) of a file or buffer
bool matchingFileHeader(const char *path, const uint8_t *header, size_t length);
bool matchingBufferHeader(const uint8_t *buffer, const uint8_t *header, size_t length);


//
// Controlling time
//

// Puts the current thread to sleep for a given amout of micro seconds
void sleepMicrosec(unsigned usec);

/* Sleeps until the kernel timer reaches kernelTargetTime
 *
 * kernelEarlyWakeup: To increase timing precision, the function
 *                    wakes up the thread earlier by this amount and waits
 *                    actively in a delay loop until the deadline is reached.
 *
 * Returns the overshoot time (jitter), measured in kernel time units. Smaller
 * values are better, 0 is best.
 */
int64_t sleepUntil(uint64_t kernelTargetTime, uint64_t kernelEarlyWakeup);


//
// Computing checksums
//

// Returns the FNV-1a seed value
inline uint32_t fnv_1a_init32() { return 0x811c9dc5; }
inline uint64_t fnv_1a_init64() { return 0xcbf29ce484222325; }

// Performs a single iteration of the FNV-1a hash algorithm
inline uint32_t fnv_1a_it32(uint32_t prev, uint32_t value) {
    return (prev ^ value) * 0x1000193; }
inline uint64_t fnv_1a_it64(uint64_t prev, uint64_t value) {
    return (prev ^ value) * 0x100000001b3; }

// Computes a FNV-1a checksum for a given buffer
uint32_t fnv_1a_32(const uint8_t *addr, size_t size);
uint64_t fnv_1a_64(const uint8_t *addr, size_t size);

// Computes a CRC-32 checksum for a given buffer
uint32_t crc32(const uint8_t *addr, size_t size);
uint32_t crc32forByte(uint32_t r); // Helper


//
// Debugging
//

// Returns true if this executable is a release build
bool releaseBuild();

#endif
