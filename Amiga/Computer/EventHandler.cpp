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
EventHandler::_dump()
{
    EventHandlerInfo info = getInfo();
    
    amiga->dumpClock();
    
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

EventSlotInfo
EventHandler::getPrimarySlotInfo(int slot)
{
    EventSlotInfo info;
    
    Cycle trigger = primSlot[slot].triggerCycle;
    FramePosition pos = amiga->dma.cycle2FramePosition(trigger);
    
    info.trigger = trigger;
    info.triggerRel = trigger - amiga->dma.clock;
    info.eventId = primSlot[slot].id;
    info.frame = pos.frame - amiga->dma.frame;
    info.vpos = pos.vpos;
    info.hpos = pos.hpos;
    
    switch ((EventSlot)slot) {
            
        case CIAA_SLOT:
            
            info.slotName = "CIA A";
            switch (primSlot[slot].id) {
                case 0:                info.eventName = "none"; break;
                case CIA_EXECUTE:      info.eventName = "CIA_EXECUTE"; break;
                case CIA_WAKEUP:       info.eventName = "CIA_WAKEUP"; break;
                default:               info.eventName = "*** INVALID ***"; break;
            }
            break;
            
        case CIAB_SLOT:
            
            info.slotName = "CIA B";
            switch (primSlot[slot].id) {
                case 0:                info.eventName = "none"; break;
                case CIA_EXECUTE:      info.eventName = "CIA_EXECUTE"; break;
                case CIA_WAKEUP:       info.eventName = "CIA_WAKEUP"; break;
                default:               info.eventName = "*** INVALID ***"; break;
            }
            break;
            
        case DMA_SLOT:
            
            info.slotName = "DMA";
            switch (primSlot[slot].id) {
                case 0:                info.eventName = "none"; break;
                case DMA_DISK:         info.eventName = "DMA_DISK"; break;
                case DMA_A0:           info.eventName = "A0"; break;
                case DMA_A1:           info.eventName = "A1"; break;
                case DMA_A2:           info.eventName = "A2"; break;
                case DMA_A3:           info.eventName = "A3"; break;
                case DMA_S0:           info.eventName = "S0"; break;
                case DMA_S1:           info.eventName = "S1"; break;
                case DMA_S2:           info.eventName = "S2"; break;
                case DMA_S3:           info.eventName = "S3"; break;
                case DMA_S4:           info.eventName = "S4"; break;
                case DMA_S5:           info.eventName = "S5"; break;
                case DMA_S6:           info.eventName = "S6"; break;
                case DMA_S7:           info.eventName = "S7"; break;
                case DMA_L1:           info.eventName = "L1"; break;
                case DMA_L2:           info.eventName = "L2"; break;
                case DMA_L3:           info.eventName = "L3"; break;
                case DMA_L4:           info.eventName = "L4"; break;
                case DMA_L5:           info.eventName = "L5"; break;
                case DMA_L6:           info.eventName = "L6"; break;
                case DMA_H1:           info.eventName = "H1"; break;
                case DMA_H2:           info.eventName = "H2"; break;
                case DMA_H3:           info.eventName = "H3"; break;
                case DMA_H4:           info.eventName = "H4"; break;
                default:               info.eventName = "*** INVALID ***"; break;
            }
            break;
            
        case COP_SLOT:
            
            info.slotName = "Copper";
            switch (primSlot[slot].id) {
                    
                case 0:                info.eventName = "none"; break;
                case COP_REQUEST_DMA:  info.eventName = "COP_REQUEST_DMA"; break;
                case COP_FETCH:        info.eventName = "COP_FETCH"; break;
                case COP_MOVE:         info.eventName = "COP_MOVE"; break;
                case COP_WAIT_OR_SKIP: info.eventName = "WAIT_OR_SKIP"; break;
                case COP_WAIT:         info.eventName = "COP_WAIT"; break;
                case COP_SKIP:         info.eventName = "COP_SKIP"; break;
                case COP_JMP1:         info.eventName = "COP_JMP1"; break;
                case COP_JMP2:         info.eventName = "COP_JMP2"; break;
                default:               info.eventName = "*** INVALID ***"; break;
            }
            break;
            
        case BLT_SLOT:
            
            info.slotName = "Blitter";
            switch (primSlot[slot].id) {
                    
                case 0:                info.eventName = "none"; break;
                case BLT_INIT:         info.eventName = "BLT_INIT"; break;
                case BLT_EXECUTE:      info.eventName = "BLT_EXECUTE"; break;
                case BLT_FAST_BLIT:    info.eventName = "BLT_FAST_BLIT"; break;
                default:               info.eventName = "*** INVALID ***"; break;
            }
            break;
            
        case RAS_SLOT:
            
            info.slotName = "Raster";
            switch (primSlot[slot].id) {
                    
                case 0:                info.eventName = "none"; break;
                case RAS_HSYNC:        info.eventName = "RAS_HSYNC"; break;
                case RAS_DIWSTRT:      info.eventName = "RAS_DIWSTRT"; break;
                case RAS_DIWDRAW:      info.eventName = "RAS_DIWDRAW"; break;
                default:               info.eventName = "*** INVALID ***"; break;
            }
            break;
            
        case SEC_SLOT:
            
            info.slotName = "Secondary";
            switch (primSlot[slot].id) {
                    
                case 0:                info.eventName = "none"; break;
                case SEC_TRIGGER:      info.eventName = "SEC_TRIGGER"; break;
                default:               info.eventName = "*** INVALID ***"; break;
            }
            break;
            
        default: assert(false);
    }
    
    return info;
}

EventSlotInfo
EventHandler::getSecondarySlotInfo(int slot)
{
    EventSlotInfo info;
    
    Cycle trigger = secSlot[slot].triggerCycle;
    FramePosition pos = amiga->dma.cycle2FramePosition(trigger);
    
    info.trigger = trigger;
    info.triggerRel = trigger - amiga->dma.clock;
    info.eventId = secSlot[slot].id;
    info.frame = pos.frame - amiga->dma.frame;
    info.vpos = pos.vpos;
    info.hpos = pos.hpos;
    
    switch ((EventSlot)slot) {
            
        case TBE_IRQ_SLOT:       info.slotName = "Serial Output IRQ"; break;
        case DSKBLK_IRQ_SLOT:    info.slotName = "Disk DMA IRQ"; break;
        case SOFT_IRQ_SLOT:      info.slotName = "Software IRQ"; break;
        case PORTS_IRQ_SLOT:     info.slotName = "CIA A IRQ"; break;
        case COPR_IRQ_SLOT:      info.slotName = "Copper IRQ"; break;
        case VERTB_IRQ_SLOT:     info.slotName = "VBlank IRQ"; break;
        case BLIT_IRQ_SLOT:      info.slotName = "Blitter IRQ"; break;
        case AUD0_IRQ_SLOT:      info.slotName = "Audio 0 IRQ"; break;
        case AUD1_IRQ_SLOT:      info.slotName = "Audio 1 IRQ"; break;
        case AUD2_IRQ_SLOT:      info.slotName = "Audio 2 IRQ"; break;
        case AUD3_IRQ_SLOT:      info.slotName = "Audio 3 IRQ"; break;
        case RBF_IRQ_SLOT:       info.slotName = "Serial Input IRQ"; break;
        case DSKSYN_IRQ_SLOT:    info.slotName = "Disk Sync IRQ"; break;
        case EXTER_IRQ_SLOT:     info.slotName = "CIA B IRQ"; break;
        default:                 info.slotName = "*** INVALID ***"; break;
    }
    
    switch ((EventSlot)slot) {
            
        case TBE_IRQ_SLOT:
        case DSKBLK_IRQ_SLOT:
        case SOFT_IRQ_SLOT:
        case PORTS_IRQ_SLOT:
        case COPR_IRQ_SLOT:
        case VERTB_IRQ_SLOT:
        case BLIT_IRQ_SLOT:
        case AUD0_IRQ_SLOT:
        case AUD1_IRQ_SLOT:
        case AUD2_IRQ_SLOT:
        case AUD3_IRQ_SLOT:
        case RBF_IRQ_SLOT:
        case DSKSYN_IRQ_SLOT:
        case EXTER_IRQ_SLOT:
            
            switch (secSlot[slot].id) {
                    
                case 0:          info.eventName = "none"; break;
                case IRQ_SET:    info.eventName = "IRQ_SET"; break;
                case IRQ_CLEAR:  info.eventName = "IRQ_CLEAR"; break;
                default:         info.eventName = "*** INVALID ***"; break;
            }
            break;
            
        default: assert(false);
    }

    return info;
}

EventHandlerInfo
EventHandler::getInfo()
{
    EventHandlerInfo info;

    info.dmaClock = amiga->dma.clock;

    // Primary events
    for (unsigned i = 0; i < PRIM_SLOT_COUNT; i++)
        info.primary[i] = getPrimarySlotInfo(i);
    
    // Secondary events
    for (unsigned i = 0; i < SEC_SLOT_COUNT; i++)
        info.secondary[i] = getSecondarySlotInfo(i);
    
    return info;
}

void
EventHandler::_executeUntil(Cycle cycle) {
    
    // Check for a CIA A event
    if (isDue(CIAA_SLOT, cycle)) {
        
        assert(checkTriggeredEvent(CIAA_SLOT));
        
        switch(primSlot[CIAA_SLOT].id) {
                
            case CIA_EXECUTE:
                amiga->ciaA.executeOneCycle();
                break;
                
            case CIA_WAKEUP:
                amiga->ciaA.wakeUp();
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
                amiga->ciaB.executeOneCycle();
                break;
                
            case CIA_WAKEUP:
                amiga->ciaB.wakeUp();
                break;
                
            default:
                assert(false);
        }
    }
    
    // Check for a bitplane event
    if (isDue(DMA_SLOT, cycle)) {
        assert(checkTriggeredEvent(DMA_SLOT));
        amiga->dma.serviceDMAEvent(primSlot[DMA_SLOT].id);
    }
    
    // Check for a Copper event
    if (isDue(COP_SLOT, cycle)) {
        assert(checkTriggeredEvent(COP_SLOT));
        amiga->dma.copper.serviceEvent(primSlot[COP_SLOT].id);
    }
    
    // Check for a Blitter event
    if (isDue(BLT_SLOT, cycle)) {
        assert(checkTriggeredEvent(BLT_SLOT));
        amiga->dma.blitter.serviceEvent(primSlot[BLT_SLOT].id);
    }
    
    // Check for a raster event
    if (isDue(RAS_SLOT, cycle)) {
        assert(checkTriggeredEvent(RAS_SLOT));
        amiga->dma.serviceRASEvent(primSlot[RAS_SLOT].id);
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
    if (isDueSec(TBE_IRQ_SLOT, cycle)) {
        serveIRQEvent(TBE_IRQ_SLOT, 0);
    }
    if (isDueSec(DSKBLK_IRQ_SLOT, cycle)) {
        serveIRQEvent(DSKBLK_IRQ_SLOT, 1);
    }
    if (isDueSec(SOFT_IRQ_SLOT, cycle)) {
        serveIRQEvent(SOFT_IRQ_SLOT, 2);
    }
    if (isDueSec(PORTS_IRQ_SLOT, cycle)) {
        serveIRQEvent(PORTS_IRQ_SLOT, 3);
    }
    if (isDueSec(COPR_IRQ_SLOT, cycle)) {
        serveIRQEvent(COPR_IRQ_SLOT, 4);
    }
    if (isDueSec(VERTB_IRQ_SLOT, cycle)) {
        serveIRQEvent(VERTB_IRQ_SLOT, 5);
    }
    if (isDueSec(BLIT_IRQ_SLOT, cycle)) {
        serveIRQEvent(BLIT_IRQ_SLOT, 6);
    }
    if (isDueSec(AUD0_IRQ_SLOT, cycle)) {
        serveIRQEvent(AUD0_IRQ_SLOT, 7);
    }
    if (isDueSec(AUD1_IRQ_SLOT, cycle)) {
        serveIRQEvent(AUD1_IRQ_SLOT, 8);
    }
    if (isDueSec(AUD2_IRQ_SLOT, cycle)) {
        serveIRQEvent(AUD2_IRQ_SLOT, 9);
    }
    if (isDueSec(AUD3_IRQ_SLOT, cycle)) {
        serveIRQEvent(AUD3_IRQ_SLOT, 10);
    }
    if (isDueSec(RBF_IRQ_SLOT, cycle)) {
        serveIRQEvent(RBF_IRQ_SLOT, 11);
    }
    if (isDueSec(DSKSYN_IRQ_SLOT, cycle)) {
        serveIRQEvent(DSKSYN_IRQ_SLOT, 12);
    }
    if (isDueSec(EXTER_IRQ_SLOT, cycle)) {
        serveIRQEvent(EXTER_IRQ_SLOT, 13);
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
    
    cycle += amiga->dma.clock;
    
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
    
    Cycle cycle = amiga->dma.beamToCyclesAbs(vpos, hpos);
    
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
    
    cycle += amiga->dma.clock;
    
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
    if (cycle < primSlot[SEC_SLOT].triggerCycle)
        rescheduleAbs(SEC_SLOT, cycle);
}

void
EventHandler::scheduleSecRel(EventSlot s, Cycle cycle, EventID id)
{
    assert(isSecondarySlot(s));
    
    cycle += amiga->dma.clock;
    
    // Schedule event in secondary table
    secSlot[s].triggerCycle = cycle;
    secSlot[s].id = id;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    if (cycle < primSlot[SEC_SLOT].triggerCycle)
        rescheduleAbs(SEC_SLOT, cycle);
}

void
EventHandler::scheduleSecPos(EventSlot s, int16_t vpos, int16_t hpos, EventID id)
{
    assert(isSecondarySlot(s));
    assert(isVposHpos(vpos, hpos));
    
    Cycle cycle = amiga->dma.beamToCyclesAbs(vpos, hpos);
    
    secSlot[s].triggerCycle = cycle;
    secSlot[s].id = id;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
}

void
EventHandler::rescheduleSecAbs(EventSlot s, Cycle cycle)
{
    assert(isSecondarySlot(s));
    
    secSlot[s].triggerCycle = cycle;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
}

void
EventHandler::rescheduleSecRel(EventSlot s, Cycle cycle)
{
    assert(isSecondarySlot(s));
    
    cycle += amiga->dma.clock;
    
    secSlot[s].triggerCycle = cycle;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
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
            amiga->paula.setINTREQ(0x8000 | (1 << irqBit));
            break;
            
        case IRQ_CLEAR:
            amiga->paula.setINTREQ(1 << irqBit);
            break;
            
        default:
            assert(false);
    }
    
    cancelSec(s);
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
    if (amiga->dma.clock != primSlot[s].triggerCycle) {
        return true;
    }
    
    return true;
}
