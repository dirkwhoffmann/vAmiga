// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "utl/abilities/Dumpable.h"
#include "utl/support/Strings.h"

namespace utl {

/*
 void
 Dumpable::hexdump(const u8 *p, isize size, isize cols, isize pad) const
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

 void
 Dumpable::hexdump(const u8 *p, isize size, isize cols) const
 {
 hexdump(p, size, cols, cols);
 }

 void
 Dumpable::hexdumpWords(const u8 *p, isize size, isize cols) const
 {
 hexdump(p, size, cols, 2);
 }

 void
 Dumpable::hexdumpLongwords(const u8 *p, isize size, isize cols) const
 {
 hexdump(p, size, cols, 4);
 }
 */

void
Dumpable::dump(std::ostream &os, const DumpOpt &opt, DataProvider reader)
{
    string fmt;

    // Assemble the format string
    auto s = opt.size == 4 ? "%l " : opt.size == 2 ? "%w " : "%b ";
    if (opt.offset) fmt += "%p:  ";
    if (opt.base)   fmt += repeat(s, opt.columns / 2) + " " + repeat(s, opt.columns / 2) + " ";
    if (opt.ascii)  fmt += "|" + repeat("%c", opt.columns * opt.size) + "|";
    fmt += "\n";

    dump(os, opt, reader, fmt.c_str());
}

void
Dumpable::dump(std::ostream &os, const DumpOpt &opt, DataProvider reader, const char *fmt)
{
    bool ctrl = false;
    isize ccnt = 0, bcnt = 0;
    char c;

    std::stringstream ss;

    auto out = [&](isize value, isize size, isize base = 0) {

        base = base ? base : opt.base;

        int w = 0;
        if (base == 8)  w += size == 1 ? 3 : size == 2 ? 6 : 11;
        if (base == 10) w += size == 1 ? 3 : size == 2 ? 5 : 10;
        if (base == 16) w += size == 1 ? 2 : size == 2 ? 4 : 8;

        if (value >= 0) {

            if (base == 8)  ss << std::setw(w) << std::setfill(' ') << std::oct << value;
            if (base == 10) ss << std::setw(w) << std::setfill(' ') << std::dec << value;
            if (base == 16) ss << std::setw(w) << std::setfill('0') << std::hex << value << std::setfill(' ');

        } else {

            ss << std::setw(w) << " ";
        }
    };

    // Continue as long as data is available
    while (reader(bcnt, 1) != -1 && reader(ccnt, 1) != -1) {

        // Rewind to the beginning of the format string
        const char *p = fmt;

        // Print one line of data
        while ((c = *p++) != '\0') {

            if (!ctrl) {

                if (c == '%') { ctrl = true; } else {
                    if (c == '\n') { ss << std::endl; } else { ss << c; }
                }
                continue;
            }

            switch (c) {

                case 'p': // Offset
                {
                    out(std::max(bcnt, ccnt), 4, 16);
                    // ss << std::setw(5) << std::setfill(' ') << std::dec << std::max(bcnt, ccnt);
                    break;
                }
                case 'a': // Character
                {
                    if (auto val = reader(ccnt, 1); val != -1) {
                        ss << (val == '\n' || isprint(int(val)) ? (char)val : ' ');
                        ccnt += 1;
                    } else {
                        ss << ' ';
                    }
                    break;
                }
                case 'c': // Character
                {
                    if (auto val = reader(ccnt, 1); val != -1) {
                        ss << (isprint(int(val)) ? (char)val : '.');
                        ccnt += 1;
                    } else {
                        ss << ' ';
                    }
                    break;
                }
                case 'b': case '1': // Byte
                {
                    auto val = reader(bcnt, 1);
                    out(val, 1);
                    bcnt += 1;
                    break;
                }
                case 'w': case '2': // Word

                {
                    auto val = reader(bcnt, 2);
                    out(val, 2);
                    bcnt += 2;
                    break;
                }
                case 'l': case '4': // Long
                {
                    auto val = reader(bcnt, 4);
                    out(val, 4);
                    bcnt += 4;
                    break;
                }
                default:
                    fatalError;
            }

            ctrl = false;
        }
    }

    // Convert the string stream into a vector
    std::vector<std::string> output;
    std::string line;
    while (std::getline(ss, line)) { output.push_back(std::move(line)); }

    // Determine the print range
    isize count = isize(output.size());
    if (opt.lines >= 0) count = std::min(count, opt.lines);
    isize start = opt.tail ? isize(output.size()) - count : 0;
    isize end = opt.tail ? isize(output.size()) : count;
    int tab = (int)std::to_string(end).size();

    // Write the requested number of lines
    for (isize i = start; i < end; i++) {
        if (opt.nr) os << std::right << std::setw(tab) << std::to_string(i) << ": ";
        os << output[i] << '\n';
    }
}

void
Dumpable::dump(std::ostream &os, const DumpOpt &opt, const u8 *buf, isize len, const char *fmt)
{
    auto read = [&](isize offset, isize bytes) {

        isize value = 0;

        while (bytes-- > 0) {

            if (offset >= len) return isize(-1);
            value = value << 8 | buf[offset++];
        }
        return value;
    };

    fmt ? dump(os, opt, read, fmt) : dump(os, opt, read);
}

/*
void
Dumpable::dump(std::ostream &os, const DumpOpt &opt, std::span<u8> span, const char *fmt)
{
    dump(os, opt, span.data(), span.size(), fmt);
}
*/

}
