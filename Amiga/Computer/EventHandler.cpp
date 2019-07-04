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
        
        { &slot,            sizeof(slot),            BYTE_ARRAY },
        { &nextTrigger,     sizeof(nextTrigger),     0 },
        { &nextSecTrigger,  sizeof(nextSecTrigger),  0 },
        
    });
}

void
EventHandler::_initialize()
{
    ciaA   = &amiga->ciaA;
    ciaB   = &amiga->ciaB;
    mem    = &amiga->mem;
    agnus  = &amiga->agnus;
    copper = &amiga->agnus.copper;
    denise = &amiga->denise;
    paula  = &amiga->paula;
}

void
EventHandler::_powerOn()
{
    // Clear the event table
    for (unsigned i = 0; i < SLOT_COUNT; i++) {
        slot[i].triggerCycle = NEVER;
        slot[i].id = (EventID)0;
        slot[i].data = 0;
    }

    // Initialize the SEC_SLOT
    scheduleAbs<SEC_SLOT>(NEVER, SEC_TRIGGER);
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
    
    info.masterClock = amiga->masterClock;
    info.dmaClock = agnus->clock;
    info.ciaAClock = ciaA->clock;
    info.ciaBClock  = ciaB->clock;
    info.frame = agnus->frame;
    info.vpos = agnus->vpos;
    info.hpos = agnus->hpos;

    // Inspect all slots
    for (int i = 0; i < SLOT_COUNT; i++) _inspectSlot((EventSlot)i);
    
    pthread_mutex_unlock(&lock);
}

void
EventHandler::_inspectSlot(EventSlot nr)
{
    assert(isEventSlot(nr));
    
    EventSlotInfo *i = &info.slotInfo[nr];
    Cycle trigger = slot[nr].triggerCycle;

    i->slotName = slotName((EventSlot)nr);
    i->eventId = slot[nr].id;
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

    switch ((EventSlot)nr) {

        case CIAA_SLOT:
        case CIAB_SLOT:

            switch (slot[nr].id) {
                case 0:             i->eventName = "none"; break;
                case CIA_EXECUTE:   i->eventName = "CIA_EXECUTE"; break;
                case CIA_WAKEUP:    i->eventName = "CIA_WAKEUP"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case DMA_SLOT:

            switch (slot[nr].id) {
                case 0:             i->eventName = "none"; break;
                case DMA_DISK:      i->eventName = "DMA_DISK"; break;
                case DMA_A0:        i->eventName = "DMA_A0"; break;
                case DMA_A1:        i->eventName = "DMA_A1"; break;
                case DMA_A2:        i->eventName = "DMA_A2"; break;
                case DMA_A3:        i->eventName = "DMA_A3"; break;
                case DMA_S0_1:      i->eventName = "DMA_S0_1"; break;
                case DMA_S1_1:      i->eventName = "DMA_S1_1"; break;
                case DMA_S2_1:      i->eventName = "DMA_S2_1"; break;
                case DMA_S3_1:      i->eventName = "DMA_S3_1"; break;
                case DMA_S4_1:      i->eventName = "DMA_S4_1"; break;
                case DMA_S5_1:      i->eventName = "DMA_S5_1"; break;
                case DMA_S6_1:      i->eventName = "DMA_S6_1"; break;
                case DMA_S7_1:      i->eventName = "DMA_S7_1"; break;
                case DMA_S0_2:      i->eventName = "DMA_S0_2"; break;
                case DMA_S1_2:      i->eventName = "DMA_S1_2"; break;
                case DMA_S2_2:      i->eventName = "DMA_S2_2"; break;
                case DMA_S3_2:      i->eventName = "DMA_S3_2"; break;
                case DMA_S4_2:      i->eventName = "DMA_S4_2"; break;
                case DMA_S5_2:      i->eventName = "DMA_S5_2"; break;
                case DMA_S6_2:      i->eventName = "DMA_S6_2"; break;
                case DMA_S7_2:      i->eventName = "DMA_S7_2"; break;
                case DMA_L1:        i->eventName = "DMA_L1"; break;
                case DMA_L1_FIRST:  i->eventName = "DMA_L1_FIRST"; break;
                case DMA_L1_LAST:   i->eventName = "DMA_L1_LAST"; break;
                case DMA_L2:        i->eventName = "DMA_L2"; break;
                case DMA_L3:        i->eventName = "DMA_L3"; break;
                case DMA_L4:        i->eventName = "DMA_L4"; break;
                case DMA_L5:        i->eventName = "DMA_L5"; break;
                case DMA_L6:        i->eventName = "DMA_L6"; break;
                case DMA_H1:        i->eventName = "DMA_H1"; break;
                case DMA_H1_FIRST:  i->eventName = "DMA_H1_FIRST"; break;
                case DMA_H1_LAST:   i->eventName = "DMA_H1_LAST"; break;
                case DMA_H2:        i->eventName = "DMA_H2"; break;
                case DMA_H3:        i->eventName = "DMA_H3"; break;
                case DMA_H4:        i->eventName = "DMA_H4"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case COP_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case COP_REQ_DMA:   i->eventName = "COP_REQ_DMA"; break;
                case COP_FETCH:     i->eventName = "COP_FETCH"; break;
                case COP_MOVE:      i->eventName = "COP_MOVE"; break;
                case COP_WAIT_SKIP: i->eventName = "WAIT_OR_SKIP"; break;
                case COP_WAIT:      i->eventName = "COP_WAIT"; break;
                case COP_SKIP:      i->eventName = "COP_SKIP"; break;
                case COP_JMP1:      i->eventName = "COP_JMP1"; break;
                case COP_JMP2:      i->eventName = "COP_JMP2"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case BLT_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case BLT_INIT:      i->eventName = "BLT_INIT"; break;
                case BLT_EXECUTE:   i->eventName = "BLT_EXECUTE"; break;
                case BLT_FAST_BLIT: i->eventName = "BLT_FAST_BLIT"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case RAS_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case RAS_HSYNC:     i->eventName = "RAS_HSYNC"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case SEC_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case SEC_TRIGGER:   i->eventName = "SEC_TRIGGER"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case DSK_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case DSK_ROTATE:    i->eventName = "DSK_ROTATE"; break;
                default:            i->eventName = "*** INVALID ***"; break;
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

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case IRQ_SET:       i->eventName = "IRQ_SET"; break;
                case IRQ_CLEAR:     i->eventName = "IRQ_CLEAR"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case REG_COP_SLOT:
        case REG_CPU_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case REG_DIWSTRT:   i->eventName = "REG_DIWSTRT"; break;
                case REG_DIWSTOP:   i->eventName = "REG_DIWSTOP"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case TXD_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case TXD_BIT:       i->eventName = "TXD_BIT"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case RXD_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case RXD_BIT:       i->eventName = "RXD_BIT"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case POT_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case POT_DISCHARGE: i->eventName = "POT_DISCHARGE"; break;
                case POT_CHARGE:    i->eventName = "POT_CHARGE"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case SYNC_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case SYNC_H:        i->eventName = "SYNC_H"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case INSPECTOR_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case INS_NONE:      i->eventName = "INS_NONE"; break;
                case INS_AMIGA:     i->eventName = "INS_AMIGA"; break;
                case INS_CPU:       i->eventName = "INS_CPU"; break;
                case INS_MEM:       i->eventName = "INS_MEM"; break;
                case INS_CIA:       i->eventName = "INS_CIA"; break;
                case INS_AGNUS:     i->eventName = "INS_AGNUS"; break;
                case INS_PAULA:     i->eventName = "INS_PAULA"; break;
                case INS_DENISE:    i->eventName = "INS_DENISE"; break;
                case INS_PORTS:     i->eventName = "INS_PORTS"; break;
                case INS_EVENTS:    i->eventName = "INS_EVENTS"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        default: assert(false);
    }
}

void
EventHandler::_dump()
{
    _inspect();
    
    amiga->dumpClock();
    
    plainmsg("Events:\n");
    for (unsigned i = 0; i < SLOT_COUNT; i++) {
        
        plainmsg("Slot: %-17s ", info.slotInfo[i].slotName);
        plainmsg("Event: %-15s ", info.slotInfo[i].eventName);
        plainmsg("Trigger: ");
        
        Cycle trigger = info.slotInfo[i].trigger;
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
EventHandler::getSlotInfo(int nr)
{
    assert(isEventSlot(nr));

    EventSlotInfo result;

    pthread_mutex_lock(&lock);
    result = info.slotInfo[nr];
    pthread_mutex_unlock(&lock);

    return result;
}

void
EventHandler::_executeUntil(Cycle cycle) {
    
    // Check for a CIA A event
    if (isDue<CIAA_SLOT>(cycle)) {
        
        assert(checkTriggeredEvent(CIAA_SLOT));
        
        switch(slot[CIAA_SLOT].id) {

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
    if (isDue<CIAB_SLOT>(cycle)) {
        
        assert(checkTriggeredEvent(CIAB_SLOT));
        
        switch(slot[CIAB_SLOT].id) {

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
    if (isDue<DMA_SLOT>(cycle)) {
        assert(checkTriggeredEvent(DMA_SLOT));
        agnus->serviceDMAEvent(slot[DMA_SLOT].id);
    }
    
    // Check for a Copper event
    if (isDue<COP_SLOT>(cycle)) {
        assert(checkTriggeredEvent(COP_SLOT));
        agnus->copper.serviceEvent(slot[COP_SLOT].id);
    }
    
    // Check for a Blitter event
    if (isDue<BLT_SLOT>(cycle)) {
        assert(checkTriggeredEvent(BLT_SLOT));
        agnus->blitter.serviceEvent(slot[BLT_SLOT].id);
    }

    // Check for a raster event
    if (isDue<RAS_SLOT>(cycle)) {
        assert(checkTriggeredEvent(RAS_SLOT));
        agnus->serviceRASEvent(slot[RAS_SLOT].id);
    }

    // Check if a secondary event needs to be processed
    if (isDue<SEC_SLOT>(cycle)) {
        _executeSecUntil(cycle);
    }

    // Determine the next trigger cycle
    nextTrigger = slot[0].triggerCycle;
    for (unsigned i = 1; i <= LAST_PRIM_SLOT; i++)
        if (slot[i].triggerCycle < nextTrigger)
            nextTrigger = slot[i].triggerCycle;
}

void
EventHandler::_executeSecUntil(Cycle cycle) {
    
    // Check all secondary event slots one by one
    if (isDue<DSK_SLOT>(cycle)) {
        paula->diskController.serveDiskEvent();
    }
    if (isDue<IRQ_TBE_SLOT>(cycle)) {
        serveIRQEvent(IRQ_TBE_SLOT, 0);
    }
    if (isDue<IRQ_DSKBLK_SLOT>(cycle)) {
        serveIRQEvent(IRQ_DSKBLK_SLOT, 1);
    }
    if (isDue<IRQ_SOFT_SLOT>(cycle)) {
        serveIRQEvent(IRQ_SOFT_SLOT, 2);
    }
    if (isDue<IRQ_PORTS_SLOT>(cycle)) {
        serveIRQEvent(IRQ_PORTS_SLOT, 3);
    }
    if (isDue<IRQ_COPR_SLOT>(cycle)) {
        serveIRQEvent(IRQ_COPR_SLOT, 4);
    }
    if (isDue<IRQ_VERTB_SLOT>(cycle)) {
        serveIRQEvent(IRQ_VERTB_SLOT, 5);
    }
    if (isDue<IRQ_BLIT_SLOT>(cycle)) {
        serveIRQEvent(IRQ_BLIT_SLOT, 6);
    }
    if (isDue<IRQ_AUD0_SLOT>(cycle)) {
        serveIRQEvent(IRQ_AUD0_SLOT, 7);
    }
    if (isDue<IRQ_AUD1_SLOT>(cycle)) {
        serveIRQEvent(IRQ_AUD1_SLOT, 8);
    }
    if (isDue<IRQ_AUD2_SLOT>(cycle)) {
        serveIRQEvent(IRQ_AUD2_SLOT, 9);
    }
    if (isDue<IRQ_AUD3_SLOT>(cycle)) {
        serveIRQEvent(IRQ_AUD3_SLOT, 10);
    }
    if (isDue<IRQ_RBF_SLOT>(cycle)) {
        serveIRQEvent(IRQ_RBF_SLOT, 11);
    }
    if (isDue<IRQ_DSKSYN_SLOT>(cycle)) {
        serveIRQEvent(IRQ_DSKSYN_SLOT, 12);
    }
    if (isDue<IRQ_EXTER_SLOT>(cycle)) {
        serveIRQEvent(IRQ_EXTER_SLOT, 13);
    }
    if (isDue<REG_COP_SLOT>(cycle)) {
        serveRegEvent(REG_COP_SLOT);
    }
    if (isDue<REG_CPU_SLOT>(cycle)) {
        serveRegEvent(REG_CPU_SLOT);
    }
    if (isDue<TXD_SLOT>(cycle)) {
        paula->uart.serveTxdEvent(slot[TXD_SLOT].id);
    }
    if (isDue<RXD_SLOT>(cycle)) {
        paula->uart.serveRxdEvent(slot[RXD_SLOT].id);
    }
    if (isDue<POT_SLOT>(cycle)) {
        paula->servePotEvent(slot[POT_SLOT].id);
    }
    if (isDue<SYNC_SLOT>(cycle)) {
        assert(slot[SYNC_SLOT].id == SYNC_H);
        agnus->serviceSYNCEvent(slot[SYNC_SLOT].id);
    }
    if (isDue<INSPECTOR_SLOT>(cycle)) {
        serveINSEvent();
    }
    
    // Determine the next trigger cycle
    nextSecTrigger = slot[FIRST_SEC_SLOT].triggerCycle;
    for (unsigned i = FIRST_SEC_SLOT + 1; i <= LAST_SEC_SLOT; i++)
        if (slot[i].triggerCycle < nextSecTrigger)
            nextSecTrigger = slot[i].triggerCycle;
    
    // Update the secondary table trigger in the primary table
    rescheduleAbs(SEC_SLOT, nextSecTrigger);
}

Cycle
EventHandler::relToCycle(Cycle cycle)
{
    return cycle + agnus->clock; 
}

Cycle
EventHandler::posToCycle(int16_t vpos, int16_t hpos)
{
    Beam beam;
    beam.y = vpos;
    beam.x = hpos;
    return agnus->beamToCycle(beam);
}

void
EventHandler::scheduleAbs(EventSlot s, Cycle cycle, EventID id)
{
    assert(isPrimarySlot(s));

    // Pass to new API (remove this function later)
    switch (s) {
        case CIAA_SLOT: scheduleAbs<CIAA_SLOT>(cycle, id); return;
        case CIAB_SLOT: scheduleAbs<CIAB_SLOT>(cycle, id); return;
        case DMA_SLOT:  scheduleAbs<DMA_SLOT>(cycle, id); return;
        case COP_SLOT:  scheduleAbs<COP_SLOT>(cycle, id); return;
        case BLT_SLOT:  scheduleAbs<BLT_SLOT>(cycle, id); return;
        case RAS_SLOT:  scheduleAbs<RAS_SLOT>(cycle, id); return;
        case SEC_SLOT:  scheduleAbs<SEC_SLOT>(cycle, id); return;
        default: assert(false);
    }

    slot[s].triggerCycle = cycle;
    slot[s].id = id;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::scheduleRel(EventSlot s, Cycle cycle, EventID id)
{
    assert(isPrimarySlot(s));
    
    cycle += agnus->clock;
    
    slot[s].triggerCycle = cycle;
    slot[s].id = id;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
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

    slot[s].triggerCycle = cycle;
    slot[s].id = id;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::rescheduleAbs(EventSlot s, Cycle cycle)
{
    assert(isPrimarySlot(s));
    
    slot[s].triggerCycle = cycle;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::rescheduleRel(EventSlot s, Cycle cycle)
{
    assert(isPrimarySlot(s));
    
    cycle += agnus->clock;
    
    slot[s].triggerCycle = cycle;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::disable(EventSlot s)
{
    assert(isPrimarySlot(s));
    slot[s].triggerCycle = NEVER;
}

void
EventHandler::cancel(EventSlot s)
{
    assert(isPrimarySlot(s));
    slot[s].id = (EventID)0;
    slot[s].triggerCycle = NEVER;
}

void
EventHandler::scheduleSecAbs(EventSlot s, Cycle cycle, EventID id)
{
    assert(isSecondarySlot(s));
    
    // Schedule event in secondary table
    slot[s].triggerCycle = cycle;
    slot[s].id = id;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    scheduleAbs(SEC_SLOT, nextSecTrigger, SEC_TRIGGER);
}

void
EventHandler::scheduleSecAbs(EventSlot s, Cycle cycle, EventID id, int64_t data)
{
    scheduleSecAbs(s, cycle, id);
    slot[s].data = data;
}

void
EventHandler::scheduleSecRel(EventSlot s, Cycle cycle, EventID id)
{
    assert(isSecondarySlot(s));
    
    cycle += agnus->clock;
    
    // Schedule event in secondary table
    slot[s].triggerCycle = cycle;
    slot[s].id = id;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    scheduleAbs(SEC_SLOT, nextSecTrigger, SEC_TRIGGER);
}

void
EventHandler::scheduleSecRel(EventSlot s, Cycle cycle, EventID id, int64_t data)
{
    scheduleSecRel(s, cycle, id);
    slot[s].data = data;
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

    slot[s].triggerCycle = cycle;
    slot[s].id = id;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    scheduleAbs(SEC_SLOT, nextSecTrigger, SEC_TRIGGER);
}

void
EventHandler::scheduleSecPos(EventSlot s, int16_t vpos, int16_t hpos, EventID id, int64_t data)
{
    scheduleSecPos(s, vpos, hpos, id);
    slot[s].data = data;
}

void
EventHandler::rescheduleSecAbs(EventSlot s, Cycle cycle)
{
    assert(isSecondarySlot(s));
    
    slot[s].triggerCycle = cycle;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    scheduleAbs(SEC_SLOT, nextSecTrigger, SEC_TRIGGER);
}

void
EventHandler::rescheduleSecRel(EventSlot s, Cycle cycle)
{
    assert(isSecondarySlot(s));
    
    cycle += agnus->clock;
    
    slot[s].triggerCycle = cycle;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    scheduleAbs(SEC_SLOT, nextSecTrigger, SEC_TRIGGER);
}

void
EventHandler::disableSec(EventSlot s)
{
    assert(isSecondarySlot(s));
    slot[s].triggerCycle = NEVER;
}

void
EventHandler::cancelSec(EventSlot s)
{
    assert(isSecondarySlot(s));
    slot[s].id = (EventID)0;
    slot[s].triggerCycle = NEVER;
}

void
EventHandler::serveIRQEvent(EventSlot s, int irqBit)
{
    switch (slot[s].id) {

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
EventHandler::scheduleRegEvent(EventSlot slot, Cycle cycle, EventID id, int64_t data)
{
    /* Here is the thing: A Copper write can occur every fourth cycle and
     * most writes are delayed by four cycles as well. Hence, this function
     * may be called under situations where a pending event is still in the
     * slot.
     * We solve this by serving the pending event first. But beware: We'll
     * probably run into problem with this approach if the old event is not
     * due yet. If such a situation really arises, we need to come up with a
     * different design. For example, we could add a second reg write slot for
     * the Copper and the CPU and schedule the event in any these two slots (we
     * can choose any of them as long as it is free). Alternatively, we could
     * add seperate event slots for each OCS register. However, this would blow
     * up the size of the secondary table and might therefore be a bad idea.
     */
    switch (slot) {

        case REG_COP_SLOT:
            if (hasEvent<REG_COP_SLOT>()) {
                assert(isDue<REG_COP_SLOT>(amiga->masterClock));
                serveRegEvent(slot);
            }
            break;

        case REG_CPU_SLOT:
            if (hasEvent<REG_CPU_SLOT>()) {
                assert(isDue<REG_CPU_SLOT>(amiga->masterClock));
                serveRegEvent(slot);
            }
            break;

        default:
            assert(false);
    }

    // Schedule event
    scheduleSecRel(slot, cycle, id, data);
}

void
EventHandler::serveRegEvent(EventSlot nr)
{
    EventID id = slot[nr].id;
    uint16_t data = (uint16_t)slot[nr].data;

    // debug("serveRegEvent(%d)\n", slot);

    switch (id) {

        case REG_DIWSTRT:
            agnus->setDIWSTRT((uint16_t)data);
            break;

        case REG_DIWSTOP:
            agnus->setDIWSTOP((uint16_t)data);
            break;

        default:
            assert(false);
    }

    // Remove event
    cancelSec(nr);
}

void
EventHandler::serveINSEvent()
{
    switch (slot[INSPECTOR_SLOT].id) {

        case INS_NONE:   break;
        case INS_AMIGA:  amiga->inspect(); break;
        case INS_CPU:    amiga->cpu.inspect(); break;
        case INS_MEM:    mem->inspect(); break;
        case INS_CIA:    ciaA->inspect(); ciaB->inspect(); break;
        case INS_AGNUS:  agnus->inspect(); break;
        case INS_PAULA:  paula->inspect(); break;
        case INS_DENISE: denise->inspect(); break;
        case INS_PORTS:
            amiga->serialPort.inspect();
            amiga->paula.uart.inspect();
            amiga->controlPort1.inspect();
            amiga->controlPort2.inspect();
            break;
        case INS_EVENTS: agnus->events.inspect(); break;
        default:         assert(false);
    }
    
    // Reschedule event
    rescheduleSecRel(INSPECTOR_SLOT, (Cycle)(inspectionInterval * 28000000));
}

bool
EventHandler::checkScheduledEvent(EventSlot s)
{
    assert(isPrimarySlot(s));
    
    if (slot[s].triggerCycle < 0) {
        _dump();
        panic("Scheduled event has a too small trigger cycle.");
        return false;
    }
    
    EventID id = slot[s].id;
    
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
            if (slot[s].triggerCycle != INT64_MAX && slot[s].triggerCycle % 40 != 0) {
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
    if (agnus->clock != slot[s].triggerCycle) {
        return true;
    }
    
    return true;
}
