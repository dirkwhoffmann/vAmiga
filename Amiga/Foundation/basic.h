/*!
 * @header      basic.h
 * @author      Dirk W. Hoffmann, www.dirkwhoffmann.de
 * @copyright   Dirk W. Hoffmann. All rights reserved.
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _BASIC_INC
#define _BASIC_INC

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

#include "va_std.h"




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
//! @functiongroup Handling buffers
//


//
//! @functiongroup Converting low level data objects
//

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
//! @functiongroup Handling file and path names
//

/*! @brief    Extracts filename from a path
 *  @details  Returns a newly created string. You need to delete it manually.
 */
char *extractFilename(const char *path);

/*! @brief    Extracts file suffix from a path
 *  @details  Returns a newly created string. You need to delete it manually.
 */
char *extractSuffix(const char *path);

/*! @brief    Extracts filename from a path without its suffix
 *  @details  Returns a newly created string. You need to delete it manually.
 */
char *extractFilenameWithoutSuffix(const char *path);

/*! @brief    Check file suffix
 *  @details  The function is used for determining the type of a file. 
 */
bool checkFileSuffix(const char *filename, const char *suffix);

//! @brief    Returns the size of a file in bytes
long getSizeOfFile(const char *filename);

/*! @brief    Checks the size of a file
 *  @details  The function is used for validating the size of a file.
 *  @param    filename Path and name of the file to investigate
 *  @param    min Expected minimum size (-1 if no lower bound exists)
 *  @param    max Expected maximum size (-1 if no upper bound exists)
 */
bool checkFileSizeRange(const char *filename, long min, long max);

/*! @brief    Checks the magic bytes of a file.
 *  @details  The function is used for determining the type of a file.
 *  @param    filename  Path and name of the file to investigate.
 *  @param    header    Expected byte sequence, terminated by 0x00.
 *  @return   Returns   true iff magic bytes match.
*/
bool checkFileHeader(const char *filename, const uint8_t *header);

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

/* Sleeps until kernel timer reaches kernelTargetTime
 * To increase timing precision, the function can wake up the thread earlier
 * and wait actively in a delay loop until the deadline is reached.
 * The return value is the overshoot time (jitter), measured in kernel time.
 * Smaller values are better, 0 is best.
 */
int64_t sleepUntil(uint64_t kernelTargetTime, uint64_t kernelEarlyWakeup);

#endif

//
//! @functiongroup Computing fingerprints
//

//! @brief    Computes a fingeprint based on the FNV-1a hash algorithm
uint64_t fnv_1a(uint8_t *addr, size_t size);
