#pragma once
#include <stdint.h>

namespace softfloat {

/*----------------------------------------------------------------------------
| One of the macros `BIGENDIAN' or `LITTLEENDIAN' must be defined.
*----------------------------------------------------------------------------*/
/*
#ifdef LSB_FIRST
#define LITTLEENDIAN
#else
#define BIGENDIAN
#endif
*/

/*----------------------------------------------------------------------------
| The macro `BITS64' can be defined to indicate that 64-bit integer types are
| supported by the compiler.
*----------------------------------------------------------------------------*/
#define BITS64

/*----------------------------------------------------------------------------
| Each of the following `typedef's defines the most convenient type that holds
| integers of at least as many bits as specified.  For example, `uint8' should
| be the most convenient type that can hold unsigned integers of as many as
| 8 bits.  The `flag' type must be able to hold either a 0 or 1.  For most
| implementations of C, `flag', `uint8', and `int8' should all be `typedef'ed
| to the same as `int'.
*----------------------------------------------------------------------------*/

typedef int8_t flag;
typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;

/*----------------------------------------------------------------------------
| Each of the following `typedef's defines a type that holds integers
| of _exactly_ the number of bits specified.  For instance, for most
| implementation of C, `bits16' and `sbits16' should be `typedef'ed to
| `unsigned short int' and `signed short int' (or `short int'), respectively.
*----------------------------------------------------------------------------*/
typedef uint8_t bits8;
typedef int8_t sbits8;
typedef uint16_t bits16;
typedef int16_t sbits16;
typedef uint32_t bits32;
typedef int32_t sbits32;
typedef uint64_t bits64;
typedef int64_t sbits64;

/*----------------------------------------------------------------------------
| The `LIT64' macro takes as its argument a textual integer literal and
| if necessary ``marks'' the literal as having a 64-bit integer type.
| For example, the GNU C Compiler (`gcc') requires that 64-bit literals be
| appended with the letters `LL' standing for `long long', which is `gcc's
| name for the 64-bit integer type.  Some compilers may allow `LIT64' to be
| defined as the identity macro:  `#define LIT64( a ) a'.
*----------------------------------------------------------------------------*/
#define LIT64( a ) a##ULL

/*----------------------------------------------------------------------------
| The macro `INLINE' can be used before functions that should be inlined.  If
| a compiler does not support explicit inlining, this macro should be defined
| to be `static'.
*----------------------------------------------------------------------------*/
// MAME defines INLINE


/*----------------------------------------------------------------------------
 | The macro `FLOATX80' must be defined to enable the extended double-precision
 | floating-point format `floatx80'.  If this macro is not defined, the
 | `floatx80' type will not be defined, and none of the functions that either
 | input or output the `floatx80' type will be defined.  The same applies to
 | the `FLOAT128' macro and the quadruple-precision format `float128'.
 *----------------------------------------------------------------------------*/
#define FLOATX80
#define FLOAT128

/*----------------------------------------------------------------------------
 | Software IEC/IEEE floating-point types.
 *----------------------------------------------------------------------------*/
typedef bits32 float32;
typedef bits64 float64;
#ifdef FLOATX80
typedef struct {
    bits16 high;
    bits64 low;
} floatx80;
#endif
#ifdef FLOAT128
typedef struct {
    bits64 high, low;
} float128;
#endif

}
