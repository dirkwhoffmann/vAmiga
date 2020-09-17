// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

TOD::TOD(CIA *cia, Amiga& ref) : AmigaComponent(ref)
{
    setDescription(cia->isCIAA() ? "TODA" : "TODB");
    this->cia = cia;
}

void
TOD::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    stopped = true;
    matching = true;
    tod.hi = 0x1;
}

void
TOD::_inspect()
{
    synchronized {
        
        info.value = tod;
        info.latch = latch;
        info.alarm = alarm;
    }
}

void 
TOD::_dump()
{
    msg("           Counter : %02X:%02X:%02X\n", tod.hi, tod.mid, tod.lo);
    msg("             Alarm : %02X:%02X:%02X\n", alarm.hi, alarm.mid, alarm.lo);
    msg("             Latch : %02X:%02X:%02X\n", latch.hi, latch.mid, latch.lo);
    msg("            Frozen : %s\n", frozen ? "yes" : "no");
    msg("           Stopped : %s\n", stopped ? "yes" : "no");
    msg("\n");
}

u8
TOD::getCounterHi()
{
    u8 result = frozen ? latch.hi : tod.hi;

    debug(TOD_DEBUG, "getCounterHi: %02x\n", result);
    return result;
}

u8
TOD::getCounterMid()
{
    u8 result = frozen ? latch.mid : tod.mid;
    
    debug(TOD_DEBUG, "getCounterMid: %02x\n", result);
    return result;
}

u8
TOD::getCounterLo()
{
    u8 result = frozen ? latch.lo : tod.lo;

    debug(TOD_DEBUG, "getCounterLo: %02x\n", result);
    return result;
}

u8
TOD::getAlarmHi()
{
    debug(TOD_DEBUG, "getAlarmHi: %02x\n", alarm.hi);
    return alarm.hi;
}

u8
TOD::getAlarmMid()
{
    debug(TOD_DEBUG, "getAlarmMid: %02x\n", alarm.mid);
    return alarm.mid;
}

u8
TOD::getAlarmLo()
{
    debug(TOD_DEBUG, "getAlarmLo: %02x\n", alarm.lo);
    return alarm.lo;
}

void
TOD::setCounterHi(u8 value)
{
    debug(TOD_DEBUG, "setCounterHi(%x)\n", value);
    tod.hi = value;
    checkForInterrupt();
}

void
TOD::setCounterMid(u8 value)
{
    debug(TOD_DEBUG, "setCounterMid(%x)\n", value);
    tod.mid = value;
    checkForInterrupt();
}

void
TOD::setCounterLo(u8 value)
{
    debug(TOD_DEBUG, "setCounterLo(%x)\n", value);
    tod.lo = value;
    checkForInterrupt();
}

void
TOD::setAlarmHi(u8 value)
{
    debug(TOD_DEBUG, "setAlarmHi(%x)\n", value);
    alarm.hi = value;
    checkForInterrupt();
}

void
TOD::setAlarmMid(u8 value)
{
    debug(TOD_DEBUG, "setAlarmMid(%x)\n", value);
    alarm.mid = value;
    checkForInterrupt();
}

void
TOD::setAlarmLo(u8 value)
{
    debug(TOD_DEBUG, "setAlarmLo(%x)\n", value);
    alarm.lo = value;
    checkForInterrupt();
}

void
TOD::increment()
{
    if (stopped) return;

    if (!incLoNibble(tod.lo))  goto check;
    if (!incHiNibble(tod.lo))  goto check;
    if (!incLoNibble(tod.mid)) goto check;

    if (tod.value == alarm.value) {
        debug(TOD_DEBUG, "TOD bug hits: %x:%x:%x (%d,%d)\n",
              tod.hi, tod.mid, tod.lo, frozen, stopped);
    }
    if (cia->config.todBug) checkForInterrupt();

    if (!incHiNibble(tod.mid)) goto check;
    if (!incLoNibble(tod.hi))  goto check;
    incHiNibble(tod.hi);

check:
    checkForInterrupt();
}

bool
TOD::incLoNibble(u8 &counter)
{
    if ((counter & 0x0F) < 0x0F) {
        counter += 0x01; return false;
    } else {
        counter &= 0xF0; return true;
    }
}

bool
TOD::incHiNibble(u8 &counter)
{
    if ((counter & 0xF0) < 0xF0) {
        counter += 0x10; return false;
    } else {
        counter &= 0x0F; return true;
    }
}

void
TOD::checkForInterrupt()
{
    if (!matching && tod.value == alarm.value) {
        debug(TOD_DEBUG, "TOD IRQ (%02x:%02x:%02x)\n", tod.hi, tod.mid, tod.lo);
        cia->todInterrupt();
    }
    matching = (tod.value == alarm.value);
}
