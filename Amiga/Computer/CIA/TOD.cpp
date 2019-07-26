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

size_t
TOD::_load(uint8_t *buffer)
{
    SerReader reader(buffer);

    applyToPersitentItems(reader);
    applyToResetItems(reader);

    debug(SNAP_DEBUG, "Recreated from %d bytes\n", reader.ptr - buffer);
    return reader.ptr - buffer;
}

size_t
TOD::_save(uint8_t *buffer)
{
    SerWriter writer(buffer);

    applyToPersitentItems(writer);
    applyToResetItems(writer);

    debug(SNAP_DEBUG, "Serialized to %d bytes\n", writer.ptr - buffer);
    return writer.ptr - buffer;
}

CounterInfo
TOD::getInfo()
{
    CounterInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

void
TOD::increment()
{
    if (stopped)
    return;
    
    if (++tod.lo == 0) {
        if (++tod.mid == 0) {
            ++tod.hi;
        }
    }
    
    checkForInterrupt();
}

void
TOD::checkForInterrupt()
{
    // Quote from SAE: "hack: do not trigger alarm interrupt if KS code and both
    // tod and alarm == 0. This incorrectly triggers on non-cycle exact
    // modes. Real hardware value written to ciabtod by KS is always
    // at least 1 or larger due to bus cycle delays when reading old value."
    // Needs further investigation.
    if (cia->nr == 1 /* CIA B */ && alarm.value == 0) {
        // return;
    }
    
    if (!matching && tod.value == alarm.value) {
        cia->todInterrupt();
    }
    
    matching = (tod.value == alarm.value);
}
