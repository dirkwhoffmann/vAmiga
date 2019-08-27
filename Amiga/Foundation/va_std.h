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


//
// Compiler optimizations
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

// Returns the low byte or the high byte of a uint16_t value.
#define LO_BYTE(x) (uint8_t)((x) & 0xFF)
#define HI_BYTE(x) (uint8_t)((x) >> 8)

// Returns the low word or the high word of a uint32_t value.
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

// Returns a non-zero value if the n-th bit is set in x.
#define GET_BIT(x,nr) ((x) & (1 << (nr)))

// Sets a single bit.
#define SET_BIT(x,nr) ((x) |= (1 << (nr)))

// Clears a single bit.
#define CLR_BIT(x,nr) ((x) &= ~(1 << (nr)))

// Toggles a single bit.
#define TOGGLE_BIT(x,nr) ((x) ^= (1 << (nr)))

// Sets a single bit to 0 (value == 0) or 1 (value != 0)
#define WRITE_BIT(x,nr,value) ((value) ? SET_BIT(x, nr) : CLR_BIT(x, nr))

// Copies a single bit from x to y.
#define COPY_BIT(x,y,nr) ((y) = ((y) & ~(1 << (nr)) | ((x) & (1 << (nr)))))

// Replaces the low byte in a 16-bit value
#define REPLACE_LO(x,y) (((x) & ~0x00FF) | y)

// Replaces the low byte in a 16-bit value
#define REPLACE_HI(x,y) (((x) & ~0xFF00) | ((y) << 8))

// Replaces the low word in a 32-bit value
#define REPLACE_LO_WORD(x,y) (((x) & ~0xFFFF) | y)

// Replaces the high word in a 23-bit value
#define REPLACE_HI_WORD(x,y) (((x) & ~0xFFFF0000) | ((y) << 16))

// Returns true if value is rising when switching from x to y
#define RISING_EDGE(x,y) (!(x) && (y))

// Returns true if bit n is rising when switching from x to y
#define RISING_EDGE_BIT(x,y,n) (!((x) & (1 << (n))) && ((y) & (1 << (n))))

// Returns true if value is falling when switching from x to y
#define FALLING_EDGE(x,y) ((x) && !(y))

// Returns true if bit n is falling when switching from x to y
#define FALLING_EDGE_BIT(x,y,n) (((x) & (1 << (n))) && !((y) & (1 << (n))))

// Returns true if x is an odd number
#define IS_ODD(x) ((x) & 1)

// Returns true if x is an even number
#define IS_EVEN(x) (!IS_ODD(x))

// Rounds a number up or down to the next even or odd number
#define UP_TO_NEXT_EVEN(x) ((x) + ((x) & 1))
#define DOWN_TO_NEXT_EVEN(x) ((x) & (~1))
#define UP_TO_NEXT_ODD(x) ((x) | 1)
#define DOWN_TO_NEXT_ODD(x) ((x) - !((x) & 1))


//
// Performing Amiga specific bit manipulations
//

// Casts a value into the pointer format used by the Original Chip Set (OCS)
#define OCS_PTR(x) ((x) & 0x7FFFE)

// Increases or decreases a pointer given in the OCS format by a certain value
#define INC_OCS_PTR(x,y) ((x) = OCS_PTR((x)+(y)))
#define DEC_OCS_PTR(x,y) ((x) = OCS_PTR((x)-(y)))


//
// Generating string representations for numbers
//

// Converts a number into a decimal string representation
void sprintd(char *s, int digits, uint64_t value);

// Converts a number into a hexadecimal string representation
void sprintx(char *s, int digits, uint64_t value);

// Converts a number into a binary string representation
void sprintb(char *s, int digits, uint64_t value);

// Convenience wrappers
inline void sprint8d(char *s, uint8_t value) { sprintd(s, 3, value); }
inline void sprint8x(char *s, uint8_t value) { sprintx(s, 2, value); }
inline void sprint8b(char *s, uint8_t value) { sprintb(s, 8, value); }

inline void sprint16d(char *s, uint16_t value) { sprintd(s, 5, value); }
inline void sprint16x(char *s, uint16_t value) { sprintx(s, 4, value); }
inline void sprint16b(char *s, uint16_t value) { sprintb(s, 16, value); }

inline void sprint24d(char *s, uint24_t value) { sprintd(s, 8, value); }
inline void sprint24x(char *s, uint24_t value) { sprintx(s, 6, value); }
inline void sprint24b(char *s, uint24_t value) { sprintb(s, 24, value); }

void hexdump(uint8_t *addr, size_t size);

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
 *   - header    Expected byte sequence
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
// Managing time
//

// Puts the current thread to sleep for a given amout of micro seconds.
void sleepMicrosec(unsigned usec);

/* Sleeps until the kernel timer reaches kernelTargetTime
 * - kernelEarlyWakeup To increase timing precision, the function
 *                     wakes up the thread earlier by this amount and waits
 *                     actively in a delay loop until the deadline is reached.
 * Returns the overshoot time (jitter), measured in kernel time units. Smaller
 * values are better, 0 is best.
 */
int64_t sleepUntil(uint64_t kernelTargetTime, uint64_t kernelEarlyWakeup);


//
// Computing checksums
//

// Returns the FNV-1a seed value.
inline uint32_t fnv_1a_init32() { return 0x811c9dc5; }
inline uint64_t fnv_1a_init64() { return 0xcbf29ce484222325; }

// Performs a single iteration of the FNV-1a hash algorithm.
inline uint32_t fnv_1a_it32(uint32_t prev, uint32_t value) { return (prev ^ value) * 0x1000193; }
inline uint64_t fnv_1a_it64(uint64_t prev, uint64_t value) { return (prev ^ value) * 0x100000001b3; }

// Computes a 64-bit checksum based on the FNV-1a hash algorithm.
uint64_t fnv_1a(uint8_t *addr, size_t size);


//
// Debugging
//

// Returns true if we're running a release build.
bool releaseBuild();

#endif
