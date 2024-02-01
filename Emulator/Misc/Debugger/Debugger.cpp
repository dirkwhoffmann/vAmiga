// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Debugger.h"
#include "Amiga.h"
#include "IOUtils.h"
#include <sstream>

namespace vamiga {

void
Debugger::stopAndGo()
{
    isRunning() ? amiga.pause() : amiga.run();
}

void
Debugger::stepInto()
{
    if (isRunning()) return;

    cpu.debugger.stepInto();
    amiga.run();

    // Inform the GUI
    msgQueue.put(MSG_STEP);
}

void
Debugger::stepOver()
{
    if (isRunning()) return;

    cpu.debugger.stepOver();
    amiga.run();

    // Inform the GUI
    msgQueue.put(MSG_STEP);
}

void
Debugger::convertNumeric(std::ostream& os, isize value) const
{
    using namespace util;

    auto chr = [&](u8 v) { return std::isprint(v) ? (char)v : '.'; };

    os << std::setw(10) << std::right << std::setfill(' ') << dec(u32(value)) << " | ";
    os << hex(u32(value)) << " | ";
    os << bin(u32(value)) << " | ";
    os << "\"";
    os << chr(BYTE3(value)) << chr(BYTE2(value)) << chr(BYTE1(value)) << chr(BYTE0(value));
    os << "\"";
    os << std::endl;
}

void 
Debugger::convertNumeric(std::ostream& os, string s) const
{
    u8 bytes[4];

    bytes[0] = s.length() >= 4 ? (u8)s[s.length() - 1] : 0;
    bytes[1] = s.length() >= 3 ? (u8)s[s.length() - 2] : 0;
    bytes[2] = s.length() >= 2 ? (u8)s[s.length() - 3] : 0;
    bytes[3] = s.length() >= 1 ? (u8)s[s.length() - 4] : 0;

    convertNumeric(os, HI_HI_LO_LO(bytes[0], bytes[1], bytes[2], bytes[3]));
}

}
