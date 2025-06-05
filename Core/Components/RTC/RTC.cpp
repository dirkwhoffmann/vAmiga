// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RTC.h"
#include "Chrono.h"
#include "Amiga.h"
#include "IOUtils.h"

namespace vamiga {

i64
RTC::getOption(Opt option) const
{
    switch (option) {
            
        case Opt::RTC_MODEL:  return (long)config.model;

        default:
            fatalError;
    }
}

void
RTC::checkOption(Opt option, i64 value)
{
    switch (option) {

        case Opt::RTC_MODEL:

            if (!isPoweredOff()) {
                throw AppError(Fault::OPT_LOCKED);
            }
            if (!RTCRevisionEnum::isValid(value)) {
                throw AppError(Fault::OPT_INV_ARG, RTCRevisionEnum::keyList());
            }
            return;

        default:
            throw(Fault::OPT_UNSUPPORTED);
    }
}

void
RTC::setOption(Opt option, i64 value)
{
    switch (option) {
            
        case Opt::RTC_MODEL:

            config.model = (RTCRevision)value;
            mem.updateMemSrcTables();
            return;

        default:
            fatalError;
    }
}

void
RTC::operator << (SerResetter &worker)
{
    serialize(worker);

    if (isHardResetter(worker)) {

        switch (config.model) {

            case RTCRevision::RICOH:

                reg[0][0xD] = 0b1000;
                reg[0][0xE] = 0b0000;
                reg[0][0xF] = 0b0000;
                break;

            case RTCRevision::OKI:

                reg[0][0xD] = 0b0001;
                reg[0][0xE] = 0b0000;
                reg[0][0xF] = 0b0100;
                break;

            default:
                break;
        }
    }
}

void
RTC::_dump(Category category, std::ostream &os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
        dumpConfig(os);
    }
    
    if (category == Category::State) {
        
        os << tab("timeDiff");
        os << dec(timeDiff) << std::endl;
        os << tab("lastCall");
        os << dec(lastCall) << std::endl;
        os << tab("lastMeasure");
        os << dec(lastMeasure) << std::endl;
        os << tab("lastMeasuredValue");
        os << dec(lastMeasuredValue) << std::endl;
        os << std::endl;
        
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
    
    if (timeBetweenCalls > 2 || lastCall == 0) {
        
        /* If the time between two read accesses is long, or this function is being
         * called for the first time, we compute the result based on the host machine's
         * current time and the timeDiff offset.
         */
        lastMeasure = master;
        lastMeasuredValue = (i64)time(nullptr);
        result = (time_t)lastMeasuredValue + (time_t)timeDiff;

    } else {

        /* If the time between two read accesses is short, we compute the result
         * based on the number of master-clock cycles that have elapsed since the
         * host time was last queried.
         *
         * This ensures correct RTC behavior in warp mode.For example, during boot,
         * Kickstart queries the RTC twice with a delay of over one second. If we
         * always used the host machine's current time, the reported delay would be
         * shorter than expected in warp mode.
         */
        auto elapsedTime = AS_SEC(master - lastMeasure);
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
    assert(config.model != RTCRevision::NONE);
    
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
    if (config.model == RTCRevision::NONE) return;

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
    config.model == RTCRevision::RICOH ? time2registersRicoh(&t) : time2registersOki(&t);
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
    reg[0][0xC] = (u8)(t->tm_wday);
    
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
    config.model == RTCRevision::RICOH ? registers2timeRicoh(&t) : registers2timeOki(&t);

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
