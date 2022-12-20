// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RTC.h"
#include "Chrono.h"
#include "Amiga.h"
#include "IOUtils.h"

namespace vamiga {

i64
RTC::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_RTC_MODEL:  return (long)config.model;

        default:
            fatalError;
    }
}

void
RTC::setConfigItem(Option option, i64 value)
{
    switch (option) {
            
        case OPT_RTC_MODEL:

            if (!isPoweredOff()) {
                throw VAError(ERROR_OPT_LOCKED);
            }
            if (!RTCRevisionEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, RTCRevisionEnum::keyList());
            }
            
            config.model = (RTCRevision)value;
            mem.updateMemSrcTables();
            return;

        default:
            fatalError;
    }
}

void
RTC::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {
        
        if (config.model == RTC_RICOH) {
            
            reg[0][0xD] = 0b1000;
            reg[0][0xE] = 0b0000;
            reg[0][0xF] = 0b0000;
        }
        if (config.model == RTC_OKI) {
            
            reg[0][0xD] = 0b0001;
            reg[0][0xE] = 0b0000;
            reg[0][0xF] = 0b0100;
        }
    }
}

void
RTC::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = amiga.defaults;

    std::vector <Option> options = {
        
        OPT_RTC_MODEL
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option));
    }
}

void
RTC::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
        os << tab("Chip Model");
        os << RTCRevisionEnum::key(config.model) << std::endl;
    }
    
    if (category == Category::Inspection) {
        
        for (isize i = 0; i < 16; i++) {
            os << "    " << hex((u8)i) << " : ";
            for (isize j = 0; j < 4; j++) {
                os << hex(reg[j][i]) << " ";
            }
            os << std::endl;
        }
        os << std::endl;
    }
}

time_t
RTC::getTime()
{
    Cycle result;
    Cycle master = cpu.getMasterClock();

    auto timeBetweenCalls = AS_SEC(master - lastCall);

    if (timeBetweenCalls > 2) {

        /* If the time between two read accesses is long, we compute the result
         * out of the host machine's current time and variable timeDiff.
         */
        lastMeasure = master;
        lastMeasuredValue = (i64)time(nullptr);
        result = (time_t)lastMeasuredValue + (time_t)timeDiff;

    } else {

        /* If the time between two read accesses is short, we compute the result
         * out of the master-clock cycles that have elapsed since the host
         * machine's time was queried the last time. This ensures that the
         * real-time clock behaves properly in warp mode. E.g., when the Amiga
         * boots, Kickstart tests the real-time clock by peeking the time twice
         * with a time delay of more than 1 second. If we simply query the host
         * machine's time, the time difference would be less than 1 second in
         * warp mode.
         */
        i64 elapsedTime = AS_SEC(master - lastMeasure);
        result = (time_t)lastMeasuredValue + (time_t)elapsedTime;
    }
    
    lastCall = master;
    return result;
}

void
RTC::setTime(time_t t)
{
    timeDiff = (i64)(t - time(nullptr));
}

void
RTC::update()
{
    time2registers();
}

u8
RTC::peek(isize nr)
{
    update();
    return spypeek(nr);
}

u8
RTC::spypeek(isize nr) const
{
    assert(nr < 16);
    assert(config.model != RTC_NONE);
    
    u8 result;
    
    switch (nr) {
            
        case 0xD: result = peekD(); break;
        case 0xE: result = peekE(); break;
        case 0xF: result = peekF(); break;
            
        default: // Time or date register
            
            result = reg[bank()][nr];
    }

    trace(RTC_DEBUG, "peek(%ld) = $%X [bank %ld]\n", nr, result, bank());
    return result;
}

void
RTC::poke(isize nr, u8 value)
{
    assert(nr < 16);

    trace(RTC_DEBUG, "poke(%ld, $%02X) [bank %ld]\n", nr, value, bank());

    // Ony proceed if a real-time clock is installed
    if (rtc.isPresent()) return;
    
    switch (nr) {
            
        case 0xD: pokeD(value); break;
        case 0xE: pokeE(value); break;
        case 0xF: pokeF(value); break;
            
        default: // Time or date register
            
            time2registers();
            reg[bank()][nr] = value & 0xF;
            registers2time();
    }
}

void
RTC::time2registers()
{
    // Convert the internally stored time diff to an absolute time_t value
    time_t rtcTime = getTime();
    
    // Convert the time_t value to a tm struct
    auto t = util::Time::local(rtcTime);
    
    // Write the registers
    config.model == RTC_RICOH ? time2registersRicoh(&t) : time2registersOki(&t);
}

void
RTC::time2registersOki(tm *t)
{
    reg[0][0x0] = (u8)(t->tm_sec % 10);
    reg[0][0x1] = (u8)(t->tm_sec / 10);
    reg[0][0x2] = (u8)(t->tm_min % 10);
    reg[0][0x3] = (u8)(t->tm_min / 10);
    reg[0][0x4] = (u8)(t->tm_hour % 10);
    reg[0][0x5] = (u8)(t->tm_hour / 10);
    reg[0][0x6] = (u8)(t->tm_mday % 10);
    reg[0][0x7] = (u8)(t->tm_mday / 10);
    reg[0][0x8] = (u8)((t->tm_mon + 1) % 10);
    reg[0][0x9] = (u8)((t->tm_mon + 1) / 10);
    reg[0][0xA] = (u8)(t->tm_year % 10);
    reg[0][0xB] = (u8)(t->tm_year / 10);
    reg[0][0xC] = (u8)(t->tm_yday / 7);
    
    // Change the hour format in AM/PM mode
    if (t->tm_hour > 12 && GET_BIT(reg[0][15], 2) == 0) {
        reg[0][4] = (u8)((t->tm_hour - 12) % 10);
        reg[0][5] = (u8)((t->tm_hour - 12) / 10);
        reg[0][5] |= 0b100;
    }
}

void
RTC::time2registersRicoh(tm *t)
{
    reg[0][0x0] = (u8)(t->tm_sec % 10);
    reg[0][0x1] = (u8)(t->tm_sec / 10);
    reg[0][0x2] = (u8)(t->tm_min % 10);
    reg[0][0x3] = (u8)(t->tm_min / 10);
    reg[0][0x4] = (u8)(t->tm_hour % 10);
    reg[0][0x5] = (u8)(t->tm_hour / 10);
    reg[0][0x6] = (u8)(t->tm_yday / 7);
    reg[0][0x7] = (u8)(t->tm_mday % 10);
    reg[0][0x8] = (u8)(t->tm_mday / 10);
    reg[0][0x9] = (u8)((t->tm_mon + 1) % 10);
    reg[0][0xA] = (u8)((t->tm_mon + 1) / 10);
    reg[0][0xB] = (u8)(t->tm_year % 10);
    reg[0][0xC] = (u8)(t->tm_year / 10);
    
    // Change the hour format in AM/PM mode
    if (t->tm_hour > 12 && GET_BIT(reg[0][10], 0) == 0) {
        reg[0][4] = (u8)((t->tm_hour - 12) % 10);
        reg[0][5] = (u8)((t->tm_hour - 12) / 10);
        reg[0][5] |= 0b010;
    }
    
    // Wipe out the unused bits in the alarm bank
    reg[1][0x0] &= 0b0000;
    reg[1][0x1] &= 0b0000;
    reg[1][0x2] &= 0b1111;
    reg[1][0x3] &= 0b0111;
    reg[1][0x4] &= 0b1111;
    reg[1][0x5] &= 0b0011;
    reg[1][0x6] &= 0b0111;
    reg[1][0x7] &= 0b1111;
    reg[1][0x8] &= 0b0011;
    reg[1][0x9] &= 0b0000;
    reg[1][0xA] &= 0b0001;
    reg[1][0xB] &= 0b0011;
    reg[1][0xC] &= 0b0000;
}

void
RTC::registers2time()
{
    tm t = { };
    
    // Read the registers
    config.model == RTC_RICOH ? registers2timeRicoh(&t) : registers2timeOki(&t);

    // Convert the tm struct to a time_t value
    time_t rtcTime = mktime(&t);
    
    // Update the real-time clock
    setTime(rtcTime);
}

void
RTC::registers2timeOki(tm *t)
{
    t->tm_sec  = reg[0][0x0] + 10 * reg[0][0x1];
    t->tm_min  = reg[0][0x2] + 10 * reg[0][0x3];
    t->tm_hour = reg[0][0x4] + 10 * reg[0][0x5];
    t->tm_mday = reg[0][0x6] + 10 * reg[0][0x7];
    t->tm_mon  = reg[0][0x8] + 10 * reg[0][0x9] - 1;
    t->tm_year = reg[0][0xA] + 10 * reg[0][0xB];
}

void
RTC::registers2timeRicoh(tm *t)
{
    t->tm_sec  = reg[0][0x0] + 10 * reg[0][0x1];
    t->tm_min  = reg[0][0x2] + 10 * reg[0][0x3];
    t->tm_hour = reg[0][0x4] + 10 * reg[0][0x5];
    t->tm_mday = reg[0][0x7] + 10 * reg[0][0x8];
    t->tm_mon  = reg[0][0x9] + 10 * reg[0][0xA] - 1;
    t->tm_year = reg[0][0xB] + 10 * reg[0][0xC];
}

}
