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
    setDescription(cia->nr == 0 ? "TODA" : "TODB");
    this->cia = cia;
}

void
TOD::_powerOn()
{

}

void
TOD::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.value = tod;
    info.latch = latch;
    info.alarm = alarm;
    
    pthread_mutex_unlock(&lock);
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

void
TOD::_reset()
{
    RESET_SNAPSHOT_ITEMS
    stopped = true;
    matching = true;
}

/*
CounterInfo
TOD::getInfo()
{
    CounterInfo result;
    
    if (!isRunning()) _inspect();

    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}
*/

void
TOD::increment()
{
    if (stopped) return;

    if (!incLoNibble(tod.lo))  goto check;
    if (!incHiNibble(tod.lo))  goto check;
    if (!incLoNibble(tod.mid)) goto check;

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
        cia->todInterrupt();
    }
    matching = (tod.value == alarm.value);
}
