// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/types.h"

namespace utl::abilities {

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

public:

    virtual ~Dumpable() = default;

    static void dump(std::ostream &os, const DumpOpt &opt, std::function<isize(isize,isize)>);
    static void dump(std::ostream &os, const DumpOpt &opt, std::function<isize(isize,isize)>, const char *fmt);
    static void dump(std::ostream &os, const DumpOpt &opt, u8 *buf, isize len);
    static void dump(std::ostream &os, const DumpOpt &opt, u8 *buf, isize len, const char *fmt);

    // virtual void dump(std::ostream &os, DumpOpt opt) { };
    // virtual void dump(std::ostream &os, DumpOpt opt, const char *fmt) { };
};

}
