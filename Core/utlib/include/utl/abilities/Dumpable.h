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

namespace utl {

struct DumpOpt
{
    isize base;
    isize size;
    isize prefix;
    isize columns;
    isize lines;
    bool tail;
    bool nr;
    bool offset;
    bool ascii;
};

class Dumpable {

    // Data providers (the subclass must override one of them)
    virtual std::function<isize(isize,isize)> dumpDataProvider() { return nullptr; }
    virtual std::pair<u8 *,isize> dumpDataRange() { return {nullptr,0}; }

public:

    virtual ~Dumpable() = default;

    // Prints a hex dump of a buffer to the console (DEPRECATED)
    /*
    [[deprecated]] void hexdump(const u8 *p, isize size, isize cols, isize pad) const;
    [[deprecated]] void hexdump(const u8 *p, isize size, isize cols = 32) const;
    [[deprecated]] void hexdumpWords(const u8 *p, isize size, isize cols = 32) const;
    [[deprecated]] void hexdumpLongwords(const u8 *p, isize size, isize cols = 32) const;
    */

    static void dump(std::ostream &os, const DumpOpt &opt, std::function<isize(isize,isize)>);
    static void dump(std::ostream &os, const DumpOpt &opt, std::function<isize(isize,isize)>, const char *fmt);
    static void dump(std::ostream &os, const DumpOpt &opt, const u8 *buf, isize len);
    static void dump(std::ostream &os, const DumpOpt &opt, const u8 *buf, isize len, const char *fmt);

    // Convenience wrappers
    /*
    void dump(std::ostream &os, DumpOpt opt);
    void dump(std::ostream &os, DumpOpt opt, const char *fmt);
    void hexDump(std::ostream &os);
    void memDump(std::ostream &os);
    void ascDump(std::ostream &os);
    void txtDump(std::ostream &os);
    */

    // virtual void dump(std::ostream &os, DumpOpt opt) { };

    // virtual void dump(std::ostream &os, DumpOpt opt, const char *fmt) { };
};

}
