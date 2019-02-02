/*!
 * @file        TOD.cpp
 * @author      Dirk W. Hoffmann, www.dirkwhoffmann.de
 * @copyright   Dirk W. Hoffmann. All rights reserved.
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


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
        { &matching,         sizeof(matching),         0 },
        { NULL,              0,                        0 }});
}

void
TOD::_powerOn()
{
    tod.hours = 1;
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
