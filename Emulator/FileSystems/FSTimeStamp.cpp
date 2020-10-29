// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSTimeStamp.h"

FSTimeStamp::FSTimeStamp(time_t date)
{
    const u32 secPerDay = 24 * 60 * 60;
    
    // Shift reference point from Jan 1, 1970 (Unix) to Jan 1, 1978 (Amiga)
    date -= (8 * 365 + 2) * secPerDay;
    
    // Extract components
    days = date / secPerDay;
    mins = (date % secPerDay) / 60;
    ticks = (date % secPerDay % 60) * 5 / 6;
}

void
FSTimeStamp::dump()
{
    printf("%d:%d:%d", days, mins, ticks);
}

void
FSTimeStamp::write(u8 *p)
{    
    p[0x0] = BYTE3(days);
    p[0x1] = BYTE2(days);
    p[0x2] = BYTE1(days);
    p[0x3] = BYTE0(days);
    
    p[0x4] = BYTE3(mins);
    p[0x5] = BYTE2(mins);
    p[0x6] = BYTE1(mins);
    p[0x7] = BYTE0(mins);
    
    p[0x8] = BYTE3(ticks);
    p[0x9] = BYTE2(ticks);
    p[0xA] = BYTE1(ticks);
    p[0xB] = BYTE0(ticks);
}
