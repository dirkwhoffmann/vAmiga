// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

RTC::RTC(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("RTC");
}

void
RTC::setModel(RTCModel model)
{
    debug("setModel(%d)\n", model);

    assert(isRTCModel(model));
    config.model = model;
}

void
RTC::_powerOn()
{

}

void
RTC::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS

    reg[13] = 0b001; // Control register D
    reg[14] = 0b000; // Control register E
    reg[15] = 0b100; // Control register F
}

void
RTC::_dump()
{
    msg("Model: %s\n", RTCModelName(config.model));
    for (unsigned i = 0; i < 16; i++) msg("i: %X ", reg[i]);
    msg("\n");
}

time_t
RTC::getTime()
{
    Cycle result;
    Cycle master = amiga.cpu.getMasterClock();

    long timeBetweenCalls = (master - lastCall) / 28000000;
    // debug("timeBetweenCalls = %d\n", timeBetweenCalls);
    
    if (timeBetweenCalls > 2) {

        /* If the time between two read accesses is long, we compute the result
         * of the host machine's current time and variable timeDiff.
         */
        lastMeasure = master;
        lastMeasuredValue = time(NULL);
        result = lastMeasuredValue + timeDiff;

    } else {

        /* If the time between two read accesses is short, we compute the result
         * out of the master-clock cycles that have elapsed since the host
         * machine's time was queried the last time.
         * This ensures that the real-time clock behaves properly if the
         * emulator runs in warp mode. E.g., when Kickstarts boots, it tests
         * the real-time clock by peeking the time twice with a time delay
         * of more than 1 second. If we simply query the host machine's
         * time, the time difference would be less than 1 second in warp mode.
         */
        long elapsedTime = (master - lastMeasure) / 28000000;
        result = lastMeasuredValue + elapsedTime;
    }
    
    lastCall = master;
    return result;
}

void
RTC::setTime(time_t t)
{
    timeDiff = t - time(NULL);
}

u8
RTC::peek(unsigned nr)
{
    assert(nr < 16);

    time2registers();
    u8 result = reg[nr];

    debug(RTC_DEBUG, "peek(%d) = $%X\n", result);
    return result;
}

void
RTC::poke(unsigned nr, u8 value)
{
    assert(nr < 16);
    debug(RTC_DEBUG, "poke(%d, $%02X)\n", nr, value);
    
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
    // debug("Time stamp: %s\n", asctime(t));
    
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
