// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Paula::Paula(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("Paula");

    subComponents = vector<HardwareComponent *> {
        
        &audioUnit,
        &diskController,
        &uart
    };
    
    ipl.setClock(&agnus.clock); 
}

void
Paula::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    for (int i = 0; i < 16; i++) setIntreq[i] = NEVER;
    ipl.clear();
    cpu.setIPL(0);
}

void
Paula::_inspect()
{
    synchronized {
        
        info.intreq = intreq;
        info.intena = intena;
        info.adkcon = adkcon;
    }
}

void
Paula::_dump()
{
    
}

void
Paula::_setWarp(bool enable)
{
    if (enable) {
        
        // Warping has the unavoidable drawback that audio playback gets out of
        // sync. To cope with this issue, we ramp down the volume when warping
        // is switched on and fade in smoothly when it is switched off.
        audioUnit.rampDown();
        
    } else {
        
        audioUnit.rampUp();
        audioUnit.outStream.alignWritePtr();
        // audioUnit.alignWritePtr();
    }
}

void
Paula::raiseIrq(IrqSource src)
{
    setINTREQ(true, 1 << src);
}

void
Paula::scheduleIrqAbs(IrqSource src, Cycle trigger)
{
    assert(isIrqSource(src));
    assert(trigger != 0);
    assert(agnus.slot[IRQ_SLOT].id == IRQ_CHECK);

    debug(INT_DEBUG, "scheduleIrq(%d, %d)\n", src, trigger);

    // Record the interrupt request
    if (trigger < setIntreq[src])
        setIntreq[src] = trigger;

    // Schedule the interrupt to be triggered with the proper delay
    if (trigger < agnus.slot[IRQ_SLOT].triggerCycle) {
        agnus.scheduleAbs<IRQ_SLOT>(trigger, IRQ_CHECK);
    }
}

void
Paula::scheduleIrqRel(IrqSource src, Cycle trigger)
{
    assert(trigger != 0);
    scheduleIrqAbs(src, agnus.clock + trigger);
}

void
Paula::checkInterrupt()
{
    int level = interruptLevel();
        
    if ((iplPipe & 0xFF) != level) {
    
        ipl.write((u8)level);
        iplPipe = (iplPipe & ~0xFF) | level;
                
        debug(CPU_DEBUG, "iplPipe: %016x\n", iplPipe);
        
        u8 iplValue = ipl.delayed();
        assert(iplValue == ((iplPipe >> 32) & 0xFF));
            
        agnus.scheduleRel<IPL_SLOT>(0, IPL_CHANGE, 5);
    }
}

int
Paula::interruptLevel()
{
    if (intena & 0x4000) {

        u16 mask = intreq & intena;

        if (mask & 0b0110000000000000) return 6;
        if (mask & 0b0001100000000000) return 5;
        if (mask & 0b0000011110000000) return 4;
        if (mask & 0b0000000001110000) return 3;
        if (mask & 0b0000000000001000) return 2;
        if (mask & 0b0000000000000111) return 1;
    }

    return 0;
}
