// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/common.h"
#include <span>

namespace utl {

struct DumpOpt
{
    isize base;     // 8 (Oct)  | 10 (Dec)  | 16 (Hex)
    isize size;     // 1 (Byte) |  2 (Word) |  4 (Long)
    // [[deprecated]] isize prefix;   //
    isize columns;  // number (Auto-synthesized columns if fmt is nullptr)
    isize lines;    // number (number of output lines)
    bool  tail;     // true ( list from top) | false (list from bottom)
    bool  nr;       // Add line numbers
    bool  offset;   // Add an offset column
    bool  ascii;    //
};

struct DumpFmt
{
    char   size;         // Value format 'b' = Byte, 'w' = Word, 'l' = 'Long'
    isize  columns;     // Number of data columns
    bool   nr;          // Add a column showing the current line number
    bool   offset;      // Add a column showing the current buffer offset
    bool   ascii;       // Add an ASCII column

    string fmt() const; // Translates options to a format string
};

class Dumpable {

public:

    using DataProvider = std::function<isize(isize,isize)>;

    // Derived classes must provide access to their data
    virtual DataProvider dataProvider() const = 0;

    virtual ~Dumpable() = default;

    // Class methods
    static DataProvider dataProvider(const u8 *buf, isize len);
    static DataProvider dataProvider(std::span<const u8> span);

    static void dump(std::ostream &os, DataProvider, const DumpOpt &opt, const DumpFmt &fmt);
    static void dump(std::ostream &os, DataProvider, const DumpOpt &opt, const string &fmt);



    // Instance methods
    void dump(std::ostream &os, DumpOpt opt, const char *fmt = nullptr) {
        dump(os, dataProvider(), opt, fmt);
    };
    void ascDump(std::ostream &os) {
        dump(os, { .columns = 64, .offset = true, .ascii = true });
    }
    void hexDump(std::ostream &os) {
        dump(os, { .base = 16, .columns = 64, .nr = true });
    }
    void memDump(std::ostream &os) {
        dump(os, { .base = 16, .columns = 64, .offset = true, .ascii = true });
    }
};

}
