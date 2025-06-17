// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "MemUtils.h"
#include "string.h"
#include "IOUtils.h"
#include <functional>

namespace vamiga::util {

bool isZero(const u8 *ptr, isize size)
{
    for (isize i = 0; i < size; i++) {
        if (ptr[i]) return false;
    }
    return true;
}

void replace(u8 *p, isize size, const u8 *sequence, const u8 *substitute)
{
    replace((char *)p, size, (char *)sequence, (char *)substitute);
}

void replace(char *p, isize size, const char *sequence, const char *substitute)
{
    assert(p);
    assert(sequence);
    assert(substitute);
    assert(strlen(sequence) == strlen(substitute));

    auto len = strlen(sequence);
        
    for (isize i = 0; i < size - isize(len); i++) {

        if (strncmp(p + i, sequence, len) == 0) {
            
            memcpy((void *)(p + i), (void *)substitute, len);
            return;
        }
    }
    assert(false);
}

void readAscii(const u8 *buf, isize len, char *result, char pad)
{
    assert(buf);
    assert(result);
    
    for (isize i = 0; i < len; i++) {
        
        result[i] = isprint(int(buf[i])) ? char(buf[i]) : pad;
    }
    result[len] = 0;
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

void dump(std::ostream &os, const char *fmt,
          std::function<isize(isize offset, isize bytes)> read)
{
    bool ctrl = false;
    isize ccnt = 0, bcnt = 0;
    char c;

    // Continue as long as data is available
    while (read(bcnt, 1) != -1 && read(ccnt, 1) != -1) {

        // Rewind to the beginning of the format string
        const char *p = fmt;

        // Print one line of data
        while ((c = *p++) != '\0') {

            if (!ctrl) {

                if (c == '%') { ctrl = true; } else {
                    if (c == '\n') { os << std::endl; } else { os << c; }
                }
                continue;
            }

            switch (c) {

                case 'p': // Offset

                    os << std::setw(8) << std::setfill('0') << std::dec << (u32)std::max(bcnt, ccnt);
                    break;

                case 'c': // Character

                    if (auto val = read(ccnt, 1); val != -1) {
                        os << (isprint(int(val)) ? (char)val : '.');
                        ccnt += 1;
                    } else {
                        os << ' ';
                    }
                    break;

                case 'b': // Byte

                    if (auto val = read(bcnt, 1); val != -1) {
                        os << std::setw(2) << std::setfill('0') << std::hex << val;
                        bcnt += 1;
                    } else {
                        os << std::setw(2) << std::setfill(' ') << " ";
                    }
                    break;

                case 'w': // Word

                    if (auto val = read(bcnt, 2); val != -1) {
                        os << std::setw(4) << std::setfill('0') << std::hex << val;
                        bcnt += 2;
                    } else {
                        os << std::setw(4) << std::setfill(' ') << " ";
                    }
                    break;

                case 'l': // Long

                    if (auto val = read(bcnt, 4); val != -1) {
                        os << std::setw(8) << std::setfill('0') << std::hex << val;
                        bcnt += 4;
                    } else {
                        os << std::setw(8) << std::setfill(' ') << " ";
                    }
                    break;

                default:
                    fatalError;
            }

            ctrl = false;
        }
    }
}

void dump(std::ostream &os, const char *fmt,
          std::function<isize(isize offset, isize bytes)> read, isize lines, bool tail)
{
    // Redirect output into a string stream
    std::stringstream ss;
    dump(ss, fmt, read);

    // Convert the string stream into a vector
    std::vector<std::string> output;
    std::string line;
    while (std::getline(ss, line)) { output.push_back(std::move(line)); }

    // Write the requested number of lines
    isize count = std::min((isize)output.size(), lines);
    isize start = tail ? output.size() - count : 0;
    isize end   = tail ? output.size() : count;
    for (isize i = start; i < end; i++) std::cout << output[i] << '\n';
}

void dump(std::ostream &os, const char *fmt, u8 *buf, isize len)
{
    auto read = [&](isize offset, isize bytes) {

        isize value = 0;

        while (bytes-- > 0) {

            if (offset >= len) return isize(-1);
            value = value << 8 | buf[offset++];
        }
        return value;
    };

    dump(os, fmt, read);
}

void dump(std::ostream &os, const char *fmt, u8 *buf, isize len, isize lines, bool tail)
{
    auto read = [&](isize offset, isize bytes) {

        isize value = 0;

        while (bytes-- > 0) {

            if (offset >= len) return isize(-1);
            value = value << 8 | buf[offset++];
        }
        return value;
    };

    dump(os, fmt, read, lines, tail);
}

}
