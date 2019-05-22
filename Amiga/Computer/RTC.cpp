// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

RTC::RTC()
{
    setDescription("RTC");
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &timeDiff,  sizeof(timeDiff),  0},
        { &reg,       sizeof(reg),       BYTE_ARRAY },
        { &lastCall,  sizeof(lastCall),  0},
        { &lastValue, sizeof(lastValue), 0},
    });
    
}

void
RTC::_powerOn()
{
    reg[13] = 0b001; // Control register D
    reg[14] = 0b000; // Control register E
    reg[15] = 0b100; // Control register F
}

void
RTC::_reset()
{
    
}

void
RTC::_dump()
{
    for (unsigned i = 0; i < 16; i++) {
        plainmsg("i: %X ", reg[i]);
    }
    plainmsg("\n");
}

time_t
RTC::getTime()
{
    Cycle result;
    
    long elapsedSec = (amiga->masterClock - lastCall) / 28000000;
    debug(2, "elapsedSec = %d\n", elapsedSec);
    
    /* Under normal circumstances, we compute the current time of the real-time
     * clock out of the current time of the host machine and variable timeDiff.
     */
    if (elapsedSec > 2) result = time(NULL) + timeDiff;
    
    /* If the time between two read accesses is short, we compute the current
     * time based on the elapsed cycle count. This ensures that the real-time
     * clock behaves properly if the emulator is run in warp mode.
     * When Kickstarts boots, it tests the real-time clock by peeking the time
     * two times while waiting for more than 1 sec between the two calls.
     */
    else result = lastValue + elapsedSec;
    
    lastCall = amiga->masterClock;
    lastValue = result;
    
    return result;
}

void
RTC::setTime(time_t t)
{
    timeDiff = t - time(NULL);
}

uint8_t
RTC::peek(unsigned nr)
{
    assert(nr < 16);
    
    debug(2, "Reading RTC register %d\n", nr);
    
    time2registers();
    return reg[nr];
}

void
RTC::poke(unsigned nr, uint8_t value)
{
    assert(nr < 16);
    
    debug(2, "Writing RTC register %d\n", nr);
    
    switch (nr) {
        
        case 13: // Control register D
        case 14: // Control register E
        case 15: // Control register F
        
            reg[nr] = value & 0xF;
            return;
        
        default: // Time or date register
        
            reg[nr] = value & 0xF;
            registers2time();
    }
}

void
RTC::time2registers()
{
    // Convert the internally stored time diff to an absolute time_t value.
    time_t rtcTime = getTime();
    
    // Convert the time_t value to a tm struct.
    tm *t = localtime(&rtcTime);
    debug(2, "Time stamp: %s\n", asctime(t));
    
    // Write the registers.
    
    /* 0000 (S1)   : S8   S4   S2   S1    (1-second digit register)
     * 0001 (S10)  : **** S40  S20  S10   (10-second digit register)
     * 0010 (MI1)  : mi8  mi4  mi2  mi1   (1-minute digit register)
     * 0011 (MI10) : **** mi40 mi20 mi10  (10-minute digit register)
     * 0100 (H1)   : h8   h4   h2   h1    (1-hour digit register)
     * 0101 (H10)  : **** PMAM h20  h10   (PM/AM, 10-hour digit register)
     * 0110 (D1)   : d8   d4   d2   d1    (1-day digit register)
     * 0111 (D10)  : **** **** d20  d10   (10-day digit register)
     * 1000 (MO1)  : mo8  mo4  mo2  mo1   (1-month digit register)
     * 1001 (MO10) : **** **** **** MO10  (10-month digit register)
     * 1010 (Y1)   : y8   y4   y2   y1    (1-year digit register)
     * 1011 (Y10)  : y80  y40  y20  y10   (10-year digit register)
     * 1100 (W)    : **** w4   w2   w1    (Week register)
     */
    reg[0] = t->tm_sec % 10;
    reg[1] = t->tm_sec / 10;
    reg[2] = t->tm_min % 10;
    reg[3] = t->tm_min / 10;
    reg[4] = t->tm_hour % 10;
    reg[5] = t->tm_hour / 10;
    reg[6] = t->tm_mday % 10;
    reg[7] = t->tm_mday / 10;
    reg[8] = (t->tm_mon + 1) % 10;
    reg[9] = (t->tm_mon + 1) / 10;
    reg[10] = t->tm_year % 10;
    reg[11] = t->tm_year / 10;
    reg[12] = t->tm_yday / 7;

    // Change the hour format if the 24/12 flag is cleared (AM/PM format)
    if (GET_BIT(reg[15], 2) == 0) {
        if (t->tm_hour > 12) {
            reg[4] = (t->tm_hour - 12) % 10;
            reg[5] = (t->tm_hour - 12) / 10;
            reg[5] |= 0b100;
        }
    }
}

void
RTC::registers2time()
{
    // Read the registers.
    tm t = {0};
    t.tm_sec  = reg[0] + 10 * reg[1];
    t.tm_min  = reg[2] + 10 * reg[3];
    t.tm_hour = reg[4] + 10 * reg[5];
    t.tm_mday = reg[6] + 10 * reg[7];
    t.tm_mon  = reg[8] + 10 * reg[9] - 1;
    t.tm_year = reg[10] + 10 * reg[11];
  
    // Convert the tm struct to a time_t value.
    time_t rtcTime = mktime(&t);
    
    // Update the real-time clock.
    setTime(rtcTime);
}
