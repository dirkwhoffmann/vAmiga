// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "MemUtils.h"

namespace util {

bool isZero(const u8 *ptr, usize size)
{
    for (usize i = 0; i < size; i++) {
        if (ptr[i]) return false;
    }
    return true;
}

template <isize digits> string hexstr(isize number)
{
    char str[digits + 1];

    str[digits] = 0;
    for (isize i = digits - 1; i >= 0; i--, number >>= 4) {

        auto nibble = number & 0xF;
        str[i] = (char)(nibble < 10 ? '0' + nibble : 'a' + (nibble - 10));
    }

    return string(str, digits);
}

void hexdump(u8 *p, isize size, isize cols, isize pad)
{
    while (size) {
        
        isize cnt = std::min(size, cols);
        for (isize x = 0; x < cnt; x++) {
            fprintf(stderr, "%02X %s", p[x], ((x + 1) % pad) == 0 ? " " : "");
        }
        
        size -= cnt;
        p += cnt;
        
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void hexdump(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, cols);
}

void hexdumpWords(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, 2);
}

void hexdumpLongwords(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, 4);
}

}

template string util::hexstr <1> (isize number);
template string util::hexstr <2> (isize number);
template string util::hexstr <4> (isize number);
template string util::hexstr <6> (isize number);
template string util::hexstr <8> (isize number);
template string util::hexstr <16> (isize number);
template string util::hexstr <32> (isize number);
template string util::hexstr <64> (isize number);
