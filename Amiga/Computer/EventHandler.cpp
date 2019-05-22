// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

EventHandler::EventHandler()
{
    setDescription("EventHandler");
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &primSlot,        sizeof(primSlot),        BYTE_ARRAY },
        { &nextPrimTrigger, sizeof(nextPrimTrigger), 0 },
        { &secSlot,         sizeof(secSlot),         BYTE_ARRAY },
        { &nextSecTrigger,  sizeof(nextSecTrigger),  0 },
        
    });
}

void
EventHandler::_initialize()
{
    ciaA   = &_amiga->ciaA;
    ciaB   = &_amiga->ciaB;
    agnus  = &_amiga->agnus;
    denise = &_amiga->denise;
    paula  = &_amiga->paula;
}

void
EventHandler::_powerOn()
{
    // Wipe out the primary event table
    for (unsigned i = 0; i < PRIM_SLOT_COUNT; i++) {
        primSlot[i].triggerCycle = NEVER;
        primSlot[i].id = (EventID)0;
        primSlot[i].data = 0;
    }
    
    // Wipe out the secondary event table
    for (unsigned i = 0; i < SEC_SLOT_COUNT; i++) {
        secSlot[i].triggerCycle = NEVER;
        secSlot[i].id = (EventID)0;
        secSlot[i].data = 0;
    }
    
    // Schedule the first inspection event (retriggers automatically)
    // scheduleSecRel(INSPECTOR_SLOT, 0, INS_NONE);
}

void
EventHandler::_powerOff()
{
    
}

void
EventHandler::_reset()
{
    
}

void
EventHandler::_ping()
{
    
}

void
EventHandler::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.masterClock = _amiga->masterClock;
    info.dmaClock = agnus->clock;
    info.ciaAClock = ciaA->clock;
    info.ciaBClock  = ciaB->clock;
    info.frame = agnus->frame;
    info.vpos = agnus->vpos;
    info.hpos = agnus->hpos;
    
    // Primary events
    for (unsigned i = 0; i < PRIM_SLOT_COUNT; i++)
    _inspectPrimSlot(i);
    
    // Secondary events
    for (unsigned i = 0; i < SEC_SLOT_COUNT; i++)
    _inspectSecSlot(i);
    
    pthread_mutex_unlock(&lock);
}

void
EventHandler::_inspectPrimSlot(uint32_t slot)
{
    assert(isPrimarySlot(slot));
    
    EventSlotInfo *i = &info.primary[slot];
    Cycle trigger = primSlot[slot].triggerCycle;

    i->eventId = primSlot[slot].id;
    i->trigger = trigger;
    i->triggerRel = trigger - agnus->clock;
    i->currentFrame = agnus->belongsToCurrentFrame(trigger);

    if (trigger != NEVER) {

        Beam beam = agnus->cycleToBeam(trigger);
        i->vpos = beam.y;
        i->hpos = beam.x;

    } else {

        i->vpos = 0;
        i->hpos = 0;
    }

    switch ((EventSlot)slot) {
        
        case CIAA_SLOT:
        
        i->slotName = "CIA A";
        switch (primSlot[slot].id) {
            case 0:                i->eventName = "none"; break;
            case CIA_EXECUTE:      i->eventName = "CIA_EXECUTE"; break;
            case CIA_WAKEUP:       i->eventName = "CIA_WAKEUP"; break;
            default:               i->eventName = "*** INVALID ***"; break;
        }
        break;
        
        case CIAB_SLOT:
        
        i->slotName = "CIA B";
        switch (primSlot[slot].id) {
            case 0:                i->eventName = "none"; break;
            case CIA_EXECUTE:      i->eventName = "CIA_EXECUTE"; break;
            case CIA_WAKEUP:       i->eventName = "CIA_WAKEUP"; break;
            default:               i->eventName = "*** INVALID ***"; break;
        }
        break;
        
        case DMA_SLOT:
        
        i->slotName = "DMA";
        switch (primSlot[slot].id) {
            case 0:                i->eventName = "none"; break;
            case DMA_DISK:         i->eventName = "DMA_DISK"; break;
            case DMA_A0:           i->eventName = "DMA_A0"; break;
            case DMA_A1:           i->eventName = "DMA_A1"; break;
            case DMA_A2:           i->eventName = "DMA_A2"; break;
            case DMA_A3:           i->eventName = "DMA_A3"; break;
            case DMA_S0_1:         i->eventName = "DMA_S0_1"; break;
            case DMA_S1_1:         i->eventName = "DMA_S1_1"; break;
            case DMA_S2_1:         i->eventName = "DMA_S2_1"; break;
            case DMA_S3_1:         i->eventName = "DMA_S3_1"; break;
            case DMA_S4_1:         i->eventName = "DMA_S4_1"; break;
            case DMA_S5_1:         i->eventName = "DMA_S5_1"; break;
            case DMA_S6_1:         i->eventName = "DMA_S6_1"; break;
            case DMA_S7_1:         i->eventName = "DMA_S7_1"; break;
            case DMA_S0_2:         i->eventName = "DMA_S0_2"; break;
            case DMA_S1_2:         i->eventName = "DMA_S1_2"; break;
            case DMA_S2_2:         i->eventName = "DMA_S2_2"; break;
            case DMA_S3_2:         i->eventName = "DMA_S3_2"; break;
            case DMA_S4_2:         i->eventName = "DMA_S4_2"; break;
            case DMA_S5_2:         i->eventName = "DMA_S5_2"; break;
            case DMA_S6_2:         i->eventName = "DMA_S6_2"; break;
            case DMA_S7_2:         i->eventName = "DMA_S7_2"; break;
            case DMA_L1:           i->eventName = "DMA_L1"; break;
            case DMA_L2:           i->eventName = "DMA_L2"; break;
            case DMA_L3:           i->eventName = "DMA_L3"; break;
            case DMA_L4:           i->eventName = "DMA_L4"; break;
            case DMA_L5:           i->eventName = "DMA_L5"; break;
            case DMA_L6:           i->eventName = "DMA_L6"; break;
            case DMA_H1:           i->eventName = "DMA_H1"; break;
            case DMA_H2:           i->eventName = "DMA_H2"; break;
            case DMA_H3:           i->eventName = "DMA_H3"; break;
            case DMA_H4:           i->eventName = "DMA_H4"; break;
            default:               i->eventName = "*** INVALID ***"; break;
        }
        break;
        
        case COP_SLOT:
        
        i->slotName = "Copper";
        switch (primSlot[slot].id) {
            
            case 0:                i->eventName = "none"; break;
            case COP_REQUEST_DMA:  i->eventName = "COP_REQUEST_DMA"; break;
            case COP_FETCH:        i->eventName = "COP_FETCH"; break;
            case COP_MOVE:         i->eventName = "COP_MOVE"; break;
            case COP_WAIT_OR_SKIP: i->eventName = "WAIT_OR_SKIP"; break;
            case COP_WAIT:         i->eventName = "COP_WAIT"; break;
            case COP_SKIP:         i->eventName = "COP_SKIP"; break;
            case COP_JMP1:         i->eventName = "COP_JMP1"; break;
            case COP_JMP2:         i->eventName = "COP_JMP2"; break;
            default:               i->eventName = "*** INVALID ***"; break;
        }
        break;
        
        case BLT_SLOT:
        
        i->slotName = "Blitter";
        switch (primSlot[slot].id) {
            
            case 0:                i->eventName = "none"; break;
            case BLT_INIT:         i->eventName = "BLT_INIT"; break;
            case BLT_EXECUTE:      i->eventName = "BLT_EXECUTE"; break;
            case BLT_FAST_BLIT:    i->eventName = "BLT_FAST_BLIT"; break;
            default:               i->eventName = "*** INVALID ***"; break;
        }
        break;
        
        case RAS_SLOT:
        
        i->slotName = "Raster";
        switch (primSlot[slot].id) {
            
            case 0:                i->eventName = "none"; break;
            case RAS_HSYNC:        i->eventName = "RAS_HSYNC"; break;
            case RAS_DIWSTRT:      i->eventName = "RAS_DIWSTRT"; break;
            case RAS_DIWDRAW:      i->eventName = "RAS_DIWDRAW"; break;
            default:               i->eventName = "*** INVALID ***"; break;
        }
        break;
        
        case SEC_SLOT:
        
        i->slotName = "Secondary";
        switch (primSlot[slot].id) {
            
            case 0:                i->eventName = "none"; break;
            case SEC_TRIGGER:      i->eventName = "SEC_TRIGGER"; break;
            default:               i->eventName = "*** INVALID ***"; break;
        }
        break;
        
        default: assert(false);
    }
    
}

void
EventHandler::_inspectSecSlot(uint32_t slot)
{
    assert(isSecondarySlot(slot));
    
    EventSlotInfo *i = &info.secondary[slot];
    Cycle trigger = secSlot[slot].triggerCycle;

    i->eventId = primSlot[slot].id;
    i->trigger = trigger;
    i->triggerRel = trigger - agnus->clock;
    i->currentFrame = agnus->belongsToCurrentFrame(trigger);

    if (trigger != NEVER) {

        Beam beam = agnus->cycleToBeam(trigger);
        i->vpos = beam.y;
        i->hpos = beam.x;

    } else {

        i->vpos = 0;
        i->hpos = 0;
    }

    switch ((EventSlot)slot) {
        
        case DSK_SLOT:           i->slotName = "Disk Controller"; break;
        case IRQ_TBE_SLOT:       i->slotName = "Serial Output IRQ"; break;
        case IRQ_DSKBLK_SLOT:    i->slotName = "Disk DMA IRQ"; break;
        case IRQ_SOFT_SLOT:      i->slotName = "Software IRQ"; break;
        case IRQ_PORTS_SLOT:     i->slotName = "CIA A IRQ"; break;
        case IRQ_COPR_SLOT:      i->slotName = "Copper IRQ"; break;
        case IRQ_VERTB_SLOT:     i->slotName = "VBlank IRQ"; break;
        case IRQ_BLIT_SLOT:      i->slotName = "Blitter IRQ"; break;
        case IRQ_AUD0_SLOT:      i->slotName = "Audio 0 IRQ"; break;
        case IRQ_AUD1_SLOT:      i->slotName = "Audio 1 IRQ"; break;
        case IRQ_AUD2_SLOT:      i->slotName = "Audio 2 IRQ"; break;
        case IRQ_AUD3_SLOT:      i->slotName = "Audio 3 IRQ"; break;
        case IRQ_RBF_SLOT:       i->slotName = "Serial Input IRQ"; break;
        case IRQ_DSKSYN_SLOT:    i->slotName = "Disk Sync IRQ"; break;
        case IRQ_EXTER_SLOT:     i->slotName = "CIA B IRQ"; break;
        case INSPECTOR_SLOT:     i->slotName = "Debugger"; break;
        default:                 i->slotName = "*** INVALID ***"; break;
    }
    
    switch ((EventSlot)slot) {
      
        case DSK_SLOT:
        
        switch (secSlot[slot].id) {
            
            case 0:          i->eventName = "none"; break;
            case DSK_ROTATE: i->eventName = "DSK_ROTATE"; break;
            default:         i->eventName = "*** INVALID ***"; break;
        }
        break;
        
        case IRQ_TBE_SLOT:
        case IRQ_DSKBLK_SLOT:
        case IRQ_SOFT_SLOT:
        case IRQ_PORTS_SLOT:
        case IRQ_COPR_SLOT:
        case IRQ_VERTB_SLOT:
        case IRQ_BLIT_SLOT:
        case IRQ_AUD0_SLOT:
        case IRQ_AUD1_SLOT:
        case IRQ_AUD2_SLOT:
        case IRQ_AUD3_SLOT:
        case IRQ_RBF_SLOT:
        case IRQ_DSKSYN_SLOT:
        case IRQ_EXTER_SLOT:
        
        switch (secSlot[slot].id) {
            
            case 0:          i->eventName = "none"; break;
            case IRQ_SET:    i->eventName = "IRQ_SET"; break;
            case IRQ_CLEAR:  i->eventName = "IRQ_CLEAR"; break;
            default:         i->eventName = "*** INVALID ***"; break;
        }
        break;
        
        case INSPECTOR_SLOT:
        
        switch (secSlot[slot].id) {
            
            case 0:          i->eventName = "none"; break;
            case INS_NONE:   i->eventName = "INS_NONE"; break;
            case INS_AMIGA:  i->eventName = "INS_AMIGA"; break;
            case INS_CPU:    i->eventName = "INS_CPU"; break;
            case INS_MEM:    i->eventName = "INS_MEM"; break;
            case INS_CIA:    i->eventName = "INS_CIA"; break;
            case INS_AGNUS:  i->eventName = "INS_AGNUS"; break;
            case INS_PAULA:  i->eventName = "INS_PAULA"; break;
            case INS_DENISE: i->eventName = "INS_DENISE"; break;
            case INS_EVENTS: i->eventName = "INS_EVENTS"; break;
            default:         i->eventName = "*** INVALID ***"; break;
        }
        break;
        
        default: assert(false);
    }
}

void
EventHandler::_dump()
{
    _inspect();
    
    _amiga->dumpClock();
    
    plainmsg("Primary events:\n");
    for (unsigned i = 0; i <PRIM_SLOT_COUNT; i++) {
        
        plainmsg("Slot: %-17s ", info.primary[i].slotName);
        plainmsg("Event: %-15s ", info.primary[i].eventName);
        plainmsg("Trigger: ");
        
        Cycle trigger = info.primary[i].trigger;
        if (trigger == NEVER) {
            plainmsg("never\n");
        } else {
            plainmsg("%lld ", trigger);
            plainmsg("(%lld DMA cycles away)\n", AS_DMA_CYCLES(trigger - info.dmaClock));
        }
    }
    
    plainmsg("Secondary events:\n");
    for (unsigned i = 0; i < SEC_SLOT_COUNT; i++) {
        
        plainmsg("Slot: %-17s ", info.secondary[i].slotName);
        plainmsg("Event: %-15s ", info.secondary[i].eventName);
        plainmsg("Trigger: ");
        
        Cycle trigger = info.secondary[i].trigger;
        if (trigger == NEVER) {
            plainmsg("never\n");
        } else {
            plainmsg("%lld ", trigger);
            plainmsg("(%lld DMA cycles away)\n", AS_DMA_CYCLES(trigger - info.dmaClock));
        }
    }
}

EventHandlerInfo
EventHandler::getInfo()
{
    EventHandlerInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

EventSlotInfo
EventHandler::getPrimarySlotInfo(int slot)
{
    assert(isPrimarySlot(slot));
    
    EventSlotInfo result;
    
    pthread_mutex_lock(&lock);
    result = info.primary[slot];
    pthread_mutex_unlock(&lock);
    
    return result;
}

EventSlotInfo
EventHandler::getSecondarySlotInfo(int slot)
{
    assert(isSecondarySlot(slot));
    
    EventSlotInfo result;
    
    pthread_mutex_lock(&lock);
    result = info.secondary[slot];
    pthread_mutex_unlock(&lock);
    
    return result;
}

void
EventHandler::_executeUntil(Cycle cycle) {
    
    // Check for a CIA A event
    if (isDue(CIAA_SLOT, cycle)) {
        
        assert(checkTriggeredEvent(CIAA_SLOT));
        
        switch(primSlot[CIAA_SLOT].id) {
            
            case CIA_EXECUTE:
            ciaA->executeOneCycle();
            break;
            
            case CIA_WAKEUP:
            ciaA->wakeUp();
            break;
            
            default:
            assert(false);
        }
    }
    
    // Check for a CIA B event
    if (isDue(CIAB_SLOT, cycle)) {
        
        assert(checkTriggeredEvent(CIAB_SLOT));
        
        switch(primSlot[CIAB_SLOT].id) {
            
            case CIA_EXECUTE:
            ciaB->executeOneCycle();
            break;
            
            case CIA_WAKEUP:
            ciaB->wakeUp();
            break;
            
            default:
            assert(false);
        }
    }
    
    // Check for a bitplane event
    if (isDue(DMA_SLOT, cycle)) {
        assert(checkTriggeredEvent(DMA_SLOT));
        agnus->serviceDMAEvent(primSlot[DMA_SLOT].id);
    }
    
    // Check for a Copper event
    if (isDue(COP_SLOT, cycle)) {
        assert(checkTriggeredEvent(COP_SLOT));
        agnus->copper.serviceEvent(primSlot[COP_SLOT].id);
    }
    
    // Check for a Blitter event
    if (isDue(BLT_SLOT, cycle)) {
        assert(checkTriggeredEvent(BLT_SLOT));
        agnus->blitter.serviceEvent(primSlot[BLT_SLOT].id);
    }
    
    // Check for a raster event
    if (isDue(RAS_SLOT, cycle)) {
        assert(checkTriggeredEvent(RAS_SLOT));
        agnus->serviceRASEvent(primSlot[RAS_SLOT].id);
    }
    
    // Check if a secondary event needs to be processed
    if (isDue(SEC_SLOT, cycle)) {
        _executeSecUntil(cycle);
    }
    
    // Determine the next trigger cycle
    nextPrimTrigger = primSlot[0].triggerCycle;
    for (unsigned i = 1; i < PRIM_SLOT_COUNT; i++)
    if (primSlot[i].triggerCycle < nextPrimTrigger)
    nextPrimTrigger = primSlot[i].triggerCycle;
}

void
EventHandler::_executeSecUntil(Cycle cycle) {
    
    // Check all secondary event slots one by one
    if (isDueSec(DSK_SLOT, cycle)) {
        paula->diskController.serveDiskEvent();
    }
    if (isDueSec(IRQ_TBE_SLOT, cycle)) {
        serveIRQEvent(IRQ_TBE_SLOT, 0);
    }
    if (isDueSec(IRQ_DSKBLK_SLOT, cycle)) {
        serveIRQEvent(IRQ_DSKBLK_SLOT, 1);
    }
    if (isDueSec(IRQ_SOFT_SLOT, cycle)) {
        serveIRQEvent(IRQ_SOFT_SLOT, 2);
    }
    if (isDueSec(IRQ_PORTS_SLOT, cycle)) {
        serveIRQEvent(IRQ_PORTS_SLOT, 3);
    }
    if (isDueSec(IRQ_COPR_SLOT, cycle)) {
        serveIRQEvent(IRQ_COPR_SLOT, 4);
    }
    if (isDueSec(IRQ_VERTB_SLOT, cycle)) {
        serveIRQEvent(IRQ_VERTB_SLOT, 5);
    }
    if (isDueSec(IRQ_BLIT_SLOT, cycle)) {
        serveIRQEvent(IRQ_BLIT_SLOT, 6);
    }
    if (isDueSec(IRQ_AUD0_SLOT, cycle)) {
        serveIRQEvent(IRQ_AUD0_SLOT, 7);
    }
    if (isDueSec(IRQ_AUD1_SLOT, cycle)) {
        serveIRQEvent(IRQ_AUD1_SLOT, 8);
    }
    if (isDueSec(IRQ_AUD2_SLOT, cycle)) {
        serveIRQEvent(IRQ_AUD2_SLOT, 9);
    }
    if (isDueSec(IRQ_AUD3_SLOT, cycle)) {
        serveIRQEvent(IRQ_AUD3_SLOT, 10);
    }
    if (isDueSec(IRQ_RBF_SLOT, cycle)) {
        serveIRQEvent(IRQ_RBF_SLOT, 11);
    }
    if (isDueSec(IRQ_DSKSYN_SLOT, cycle)) {
        serveIRQEvent(IRQ_DSKSYN_SLOT, 12);
    }
    if (isDueSec(IRQ_EXTER_SLOT, cycle)) {
        serveIRQEvent(IRQ_EXTER_SLOT, 13);
    }
    if (isDueSec(INSPECTOR_SLOT, cycle)) {
        serveINSEvent();
    }
    
    // Determine the next trigger cycle
    nextSecTrigger = secSlot[0].triggerCycle;
    for (unsigned i = 1; i < SEC_SLOT_COUNT; i++)
    if (secSlot[i].triggerCycle < nextSecTrigger)
    nextSecTrigger = secSlot[i].triggerCycle;
    
    // Update the secondary table trigger in the primary table
    rescheduleAbs(SEC_SLOT, nextSecTrigger);
}

void
EventHandler::scheduleAbs(EventSlot s, Cycle cycle, EventID id)
{
    assert(isPrimarySlot(s));
    
    primSlot[s].triggerCycle = cycle;
    primSlot[s].id = id;
    if (cycle < nextPrimTrigger) nextPrimTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::scheduleRel(EventSlot s, Cycle cycle, EventID id)
{
    assert(isPrimarySlot(s));
    
    cycle += agnus->clock;
    
    primSlot[s].triggerCycle = cycle;
    primSlot[s].id = id;
    if (cycle < nextPrimTrigger) nextPrimTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::schedulePos(EventSlot s, int16_t vpos, int16_t hpos, EventID id)
{
    assert(isPrimarySlot(s));
    assert(isVposHpos(vpos, hpos));

    Beam beam;
    beam.y = vpos;
    beam.x = hpos;
    Cycle cycle = agnus->beamToCycle(beam);

    primSlot[s].triggerCycle = cycle;
    primSlot[s].id = id;
    if (cycle < nextPrimTrigger) nextPrimTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::rescheduleAbs(EventSlot s, Cycle cycle)
{
    assert(isPrimarySlot(s));
    
    primSlot[s].triggerCycle = cycle;
    if (cycle < nextPrimTrigger) nextPrimTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::rescheduleRel(EventSlot s, Cycle cycle)
{
    assert(isPrimarySlot(s));
    
    cycle += agnus->clock;
    
    primSlot[s].triggerCycle = cycle;
    if (cycle < nextPrimTrigger) nextPrimTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::disable(EventSlot s)
{
    assert(isPrimarySlot(s));
    primSlot[s].triggerCycle = NEVER;
}

void
EventHandler::cancel(EventSlot s)
{
    assert(isPrimarySlot(s));
    primSlot[s].id = (EventID)0;
    primSlot[s].triggerCycle = NEVER;
}

void
EventHandler::scheduleSecAbs(EventSlot s, Cycle cycle, EventID id)
{
    assert(isSecondarySlot(s));
    
    // Schedule event in secondary table
    secSlot[s].triggerCycle = cycle;
    secSlot[s].id = id;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    scheduleAbs(SEC_SLOT, nextSecTrigger, SEC_TRIGGER);
}

void
EventHandler::scheduleSecRel(EventSlot s, Cycle cycle, EventID id)
{
    assert(isSecondarySlot(s));
    
    cycle += agnus->clock;
    
    // Schedule event in secondary table
    secSlot[s].triggerCycle = cycle;
    secSlot[s].id = id;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    scheduleAbs(SEC_SLOT, nextSecTrigger, SEC_TRIGGER);
}

void
EventHandler::scheduleSecPos(EventSlot s, int16_t vpos, int16_t hpos, EventID id)
{
    assert(isSecondarySlot(s));
    assert(isVposHpos(vpos, hpos));

    Beam beam;
    beam.y = vpos;
    beam.x = hpos;
    Cycle cycle = agnus->beamToCycle(beam);

    secSlot[s].triggerCycle = cycle;
    secSlot[s].id = id;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    scheduleAbs(SEC_SLOT, nextSecTrigger, SEC_TRIGGER);
}

void
EventHandler::rescheduleSecAbs(EventSlot s, Cycle cycle)
{
    assert(isSecondarySlot(s));
    
    secSlot[s].triggerCycle = cycle;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    scheduleAbs(SEC_SLOT, nextSecTrigger, SEC_TRIGGER);
}

void
EventHandler::rescheduleSecRel(EventSlot s, Cycle cycle)
{
    assert(isSecondarySlot(s));
    
    cycle += agnus->clock;
    
    secSlot[s].triggerCycle = cycle;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    scheduleAbs(SEC_SLOT, nextSecTrigger, SEC_TRIGGER);
}

void
EventHandler::disableSec(EventSlot s)
{
    assert(isSecondarySlot(s));
    secSlot[s].triggerCycle = NEVER;
}

void
EventHandler::cancelSec(EventSlot s)
{
    assert(isSecondarySlot(s));
    secSlot[s].id = (EventID)0;
    secSlot[s].triggerCycle = NEVER;
}

void
EventHandler::serveIRQEvent(EventSlot s, int irqBit)
{
    switch (secSlot[s].id) {
        
        case IRQ_SET:
        paula->setINTREQ(0x8000 | (1 << irqBit));
        break;
        
        case IRQ_CLEAR:
        paula->setINTREQ(1 << irqBit);
        break;
        
        default:
        assert(false);
    }
    
    cancelSec(s);
}

void
EventHandler::serveINSEvent()
{
    switch (secSlot[INSPECTOR_SLOT].id) {
        
        case INS_NONE:   break;
        case INS_AMIGA:  _amiga->inspect(); break;
        case INS_CPU:    _amiga->cpu.inspect(); break;
        case INS_MEM:    _mem->inspect(); break;
        case INS_CIA:    ciaA->inspect(); ciaB->inspect(); break;
        case INS_AGNUS:  agnus->inspect(); break;
        case INS_PAULA:  paula->inspect(); break;
        case INS_DENISE: denise->inspect(); break;
        case INS_EVENTS: agnus->eventHandler.inspect(); break;
        default:         assert(false);
    }
    
    // Reschedule event
    rescheduleSecRel(INSPECTOR_SLOT, (Cycle)(inspectionInterval * 28000000));
}

bool
EventHandler::checkScheduledEvent(EventSlot s)
{
    assert(isPrimarySlot(s));
    
    if (primSlot[s].triggerCycle < 0) {
        _dump();
        panic("Scheduled event has a too small trigger cycle.");
        return false;
    }
    
    EventID id = primSlot[s].id;
    
    if (id == 0) {
        _dump();
        panic("Event ID must not be 0.");
        return false;
    }
    
    switch (s) {
        case CIAA_SLOT:
        case CIAB_SLOT:
        if (!isCiaEvent(id)) {
            _dump();
            panic("Invalid CIA event ID.");
            return false;
        }
        if (primSlot[s].triggerCycle != INT64_MAX && primSlot[s].triggerCycle % 40 != 0) {
            _dump();
            panic("Scheduled trigger cycle is not a CIA cycle.");
            return false;
        }
        break;
        
        case DMA_SLOT:
        if (!isDmaEvent(id)) {
            _dump();
            panic("Invalid DMA event ID.");
            return false;
        }
        break;
        
        case COP_SLOT:
        if (!isCopEvent(id)) {
            _dump();
            panic("Invalid COP event ID.");
            return false;
        }
        break;
        
        case BLT_SLOT:
        if (!isBltEvent(id)) {
            _dump();
            panic("Invalid BLT event ID.");
            return false;
        }
        break;
        
        default:
        break;
    }
    return true;
}

bool
EventHandler::checkTriggeredEvent(EventSlot s)
{
    assert(isPrimarySlot(s));
    
    // Note: This function has to be called at the trigger cycle
    if (agnus->clock != primSlot[s].triggerCycle) {
        return true;
    }
    
    return true;
}
