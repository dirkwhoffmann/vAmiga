// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

TOD::TOD(CIA *cia)
{
	setDescription("TOD");
	debug(3, "    Creating TOD at address %p...\n", this);
    
    this->cia = cia;
    
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
                
        { &tod.value,        sizeof(tod.value),        0 },
        { &latch.value,      sizeof(latch.value),      0 },
        { &alarm.value,      sizeof(alarm.value),      0 },
        { &frozen,           sizeof(frozen),           0 },
        { &stopped,          sizeof(stopped),          0 },
        { &matching,         sizeof(matching),         0 }});
}

void
TOD::_powerOn()
{
    stopped = true;
}

void 
TOD::_dump()
{
	msg("                Counter : %02X:%02X:%02X\n",
        tod.minutes, tod.seconds, tod.tenth);
	msg("                  Alarm : %02X:%02X:%02X\n",
        alarm.minutes, alarm.seconds, alarm.tenth);
	msg("                  Latch : %02X:%02X:%02X\n",
        latch.minutes, latch.seconds, latch.tenth);
	msg("                 Frozen : %s\n", frozen ? "yes" : "no");
	msg("                Stopped : %s\n", stopped ? "yes" : "no");
	msg("\n");
}

TODInfo
TOD::getInfo()
{
    TODInfo info;
    
    info.time = tod;
    info.latch = latch;
    info.alarm = alarm;
    
    return info;
}

void
TOD::increment()
{
    if (stopped)
        return;
    
    if (++tod.tenth == 0) {
        if (++tod.seconds == 0) {
            ++tod.minutes;
        }
    }
    
    checkForInterrupt();
}

void
TOD::checkForInterrupt()
{
    if (!matching && tod.value == alarm.value) {
        cia->todInterrupt();
    }
    
    matching = (tod.value == alarm.value);
}
