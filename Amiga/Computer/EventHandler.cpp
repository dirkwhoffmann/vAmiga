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
}
    
void
EventHandler::_powerOn()
{
    trace = 0; // (1 << BLT_SLOT);
    
    // Wipe out the primary event table
    for (unsigned i = 0; i < EVENT_SLOT_COUNT; i++) {
        eventSlot[i].triggerCycle = NEVER;
        eventSlot[i].framePos = {0,0,0};
        eventSlot[i].id = (EventID)0;
        eventSlot[i].data = 0;
    }

    // Wipe out the secondary event table
    for (unsigned i = 0; i < SEC_SLOT_COUNT; i++) {
        secondarySlot[i].triggerCycle = NEVER;
        secondarySlot[i].framePos = {0,0,0};
        secondarySlot[i].id = (EventID)0;
        secondarySlot[i].data = 0;
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
    for (unsigned i = 0; i <primarySlotCount; i++) {

        plainmsg("Slot: %-10s ", info.primary[i].slotName);
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
    for (unsigned i = 0; i < secondarySlotCount; i++) {
        
        plainmsg("Slot: %-10s ", info.secondary[i].slotName);
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
    EventHandlerInfo info;
    
    assert(primarySlotCount == EVENT_SLOT_COUNT);
    assert(secondarySlotCount == SEC_SLOT_COUNT);

    info.dmaClock = amiga->dma.clock;

    //
    // Primary events
    //
    
    for (unsigned i = 0; i < EVENT_SLOT_COUNT; i++) {
        
        Cycle trigger = eventSlot[i].triggerCycle;
        FramePosition pos = amiga->dma.cycle2FramePosition(trigger);
        
        info.primary[i].trigger = trigger;
        info.primary[i].eventId = eventSlot[i].id;
        info.primary[i].frame = pos.frame - amiga->dma.frame;
        info.primary[i].vpos = pos.vpos;
        info.primary[i].hpos = pos.hpos;

        switch ((EventSlot)i) {
                
            case CIAA_SLOT:

                info.primary[i].slotName = "CIA A";
                switch (eventSlot[i].id) {
                    case 0:                info.primary[i].eventName = "none"; break;
                    case CIA_EXECUTE:      info.primary[i].eventName = "CIA_EXECUTE"; break;
                    case CIA_WAKEUP:       info.primary[i].eventName = "CIA_WAKEUP"; break;
                    default:               info.primary[i].eventName = "*** INVALID ***"; break;
                }
                break;

            case CIAB_SLOT:
                
                info.primary[i].slotName = "CIA B";
                switch (eventSlot[i].id) {
                    case 0:                info.primary[i].eventName = "none"; break;
                    case CIA_EXECUTE:      info.primary[i].eventName = "CIA_EXECUTE"; break;
                    case CIA_WAKEUP:       info.primary[i].eventName = "CIA_WAKEUP"; break;
                    default:               info.primary[i].eventName = "*** INVALID ***"; break;
                }
                break;
                
            case DMA_SLOT:
                
                info.primary[i].slotName = "DMA";
                switch (eventSlot[i].id) {
                    case 0:                info.primary[i].eventName = "none"; break;
                    case DMA_DISK:         info.primary[i].eventName = "DMA_DISK"; break;
                    case DMA_A0:           info.primary[i].eventName = "A0"; break;
                    case DMA_A1:           info.primary[i].eventName = "A1"; break;
                    case DMA_A2:           info.primary[i].eventName = "A2"; break;
                    case DMA_A3:           info.primary[i].eventName = "A3"; break;
                    case DMA_S0:           info.primary[i].eventName = "S0"; break;
                    case DMA_S1:           info.primary[i].eventName = "S1"; break;
                    case DMA_S2:           info.primary[i].eventName = "S2"; break;
                    case DMA_S3:           info.primary[i].eventName = "S3"; break;
                    case DMA_S4:           info.primary[i].eventName = "S4"; break;
                    case DMA_S5:           info.primary[i].eventName = "S5"; break;
                    case DMA_S6:           info.primary[i].eventName = "S6"; break;
                    case DMA_S7:           info.primary[i].eventName = "S7"; break;
                    case DMA_L1:           info.primary[i].eventName = "L1"; break;
                    case DMA_L2:           info.primary[i].eventName = "L2"; break;
                    case DMA_L3:           info.primary[i].eventName = "L3"; break;
                    case DMA_L4:           info.primary[i].eventName = "L4"; break;
                    case DMA_L5:           info.primary[i].eventName = "L5"; break;
                    case DMA_L6:           info.primary[i].eventName = "L6"; break;
                    case DMA_H1:           info.primary[i].eventName = "H1"; break;
                    case DMA_H2:           info.primary[i].eventName = "H2"; break;
                    case DMA_H3:           info.primary[i].eventName = "H3"; break;
                    case DMA_H4:           info.primary[i].eventName = "H4"; break;
                    default:               info.primary[i].eventName = "*** INVALID ***"; break;
                }
                break;
                
            case COP_SLOT:
                
                info.primary[i].slotName = "Copper";
                switch (eventSlot[i].id) {
                        
                    case 0:                info.primary[i].eventName = "none"; break;
                    case COP_REQUEST_DMA:  info.primary[i].eventName = "COP_REQUEST_DMA"; break;
                    case COP_FETCH:        info.primary[i].eventName = "COP_FETCH"; break;
                    case COP_MOVE:         info.primary[i].eventName = "COP_MOVE"; break;
                    case COP_WAIT_OR_SKIP: info.primary[i].eventName = "WAIT_OR_SKIP"; break;
                    case COP_WAIT:         info.primary[i].eventName = "COP_WAIT"; break;
                    case COP_SKIP:         info.primary[i].eventName = "COP_SKIP"; break;
                    case COP_JMP1:         info.primary[i].eventName = "COP_JMP1"; break;
                    case COP_JMP2:         info.primary[i].eventName = "COP_JMP2"; break;
                    default:               info.primary[i].eventName = "*** INVALID ***"; break;
                }
                break;
                
            case BLT_SLOT:
                
                info.primary[i].slotName = "Blitter";
                switch (eventSlot[i].id) {
                        
                    case 0:                info.primary[i].eventName = "none"; break;
                    case BLT_INIT:         info.primary[i].eventName = "BLT_INIT"; break;
                    case BLT_EXECUTE:      info.primary[i].eventName = "BLT_EXECUTE"; break;
                    case BLT_FAST_BLIT:    info.primary[i].eventName = "BLT_FAST_BLIT"; break;
                    default:               info.primary[i].eventName = "*** INVALID ***"; break;
                }
                break;
                
            case RAS_SLOT:
                
                info.primary[i].slotName = "Raster";
                switch (eventSlot[i].id) {
                        
                    case 0:                info.primary[i].eventName = "none"; break;
                    case RAS_HSYNC:        info.primary[i].eventName = "RAS_HSYNC"; break;
                    case RAS_DIWSTRT:      info.primary[i].eventName = "RAS_DIWSTRT"; break;
                    case RAS_DIWDRAW:      info.primary[i].eventName = "RAS_DIWDRAW"; break;
                    default:               info.primary[i].eventName = "*** INVALID ***"; break;
                }
                break;
                
            case SEC_SLOT:
                
                info.primary[i].slotName = "Secondary";
                switch (eventSlot[i].id) {
                        
                    case 0:                info.primary[i].eventName = "none"; break;
                    case SEC_TRIGGER:      info.primary[i].eventName = "SEC_TRIGGER"; break;
                    default:               info.primary[i].eventName = "*** INVALID ***"; break;
                }
                break;
                
            default: assert(false);
        }
    }
        
    //
    // Secondary events
    //
    
    for (unsigned i = 0; i < SEC_SLOT_COUNT; i++) {
        
        Cycle trigger = secondarySlot[i].triggerCycle;
        FramePosition pos = amiga->dma.cycle2FramePosition(trigger);
        
        info.secondary[i].trigger = trigger;
        info.secondary[i].eventId = secondarySlot[i].id;
        info.secondary[i].frame = pos.frame - amiga->dma.frame;
        info.secondary[i].vpos = pos.vpos;
        info.secondary[i].hpos = pos.hpos;

        switch ((EventSlot)i) {
                
            case TBE_IRQ_SLOT:       info.secondary[i].slotName = "TBE_IRQ"; break;
            case DSKBLK_IRQ_SLOT:    info.secondary[i].slotName = "DSKBLK_IRQ"; break;
            case SOFT_IRQ_SLOT:      info.secondary[i].slotName = "SOFT_IRQ"; break;
            case PORTS_IRQ_SLOT:     info.secondary[i].slotName = "PORTS_IRQ"; break;
            case COPR_IRQ_SLOT:      info.secondary[i].slotName = "COPR_IRQ"; break;
            case VERTB_IRQ_SLOT:     info.secondary[i].slotName = "VERTB_IRQ"; break;
            case BLIT_IRQ_SLOT:      info.secondary[i].slotName = "BLIT_IRQ"; break;
            case AUD0_IRQ_SLOT:      info.secondary[i].slotName = "AUD0_IRQ"; break;
            case AUD1_IRQ_SLOT:      info.secondary[i].slotName = "AUD1_IRQ"; break;
            case AUD2_IRQ_SLOT:      info.secondary[i].slotName = "AUD2_IRQ"; break;
            case AUD3_IRQ_SLOT:      info.secondary[i].slotName = "AUD3_IRQ"; break;
            case RBF_IRQ_SLOT:       info.secondary[i].slotName = "RBF_IRQ"; break;
            case DSKSYN_IRQ_SLOT:    info.secondary[i].slotName = "DSKSYN_IRQ"; break;
            case EXTER_IRQ_SLOT:     info.secondary[i].slotName = "EXTER_IRQ"; break;
            default:                 info.secondary[i].slotName = "*** INVALID ***"; break;
        }
        
        switch ((EventSlot)i) {
                
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
                
                switch (secondarySlot[i].id) {
                        
                    case 0:          info.secondary[i].eventName = "none"; break;
                    case IRQ_SET:    info.secondary[i].eventName = "IRQ_SET"; break;
                    case IRQ_CLEAR:  info.secondary[i].eventName = "IRQ_CLEAR"; break;
                    default:         info.secondary[i].eventName = "*** INVALID ***"; break;
                }
                break;
                
            default: assert(false);
        }
    }
    
    return info;
}

void
EventHandler::scheduleAbs(EventSlot s, Cycle cycle, EventID id)
{
    assert(isEventSlot(s));
    
    if (trace & (1 << s)) {
        debug("scheduleAbs(%d, %lld, %d)\n", s, cycle, id);
    }

    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].framePos = amiga->dma.cycle2FramePosition(cycle);
    eventSlot[s].id = id;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::scheduleRel(EventSlot s, Cycle cycle, EventID id)
{
    assert(isEventSlot(s));
    
    cycle += amiga->dma.clock;
    
    if (trace & (1 << s)) {
        debug("scheduleRel(%d, %lld, %d)\n", s, cycle, id);
    }
    
    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].framePos = amiga->dma.cycle2FramePosition(cycle);
    eventSlot[s].id = id;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::schedulePos(EventSlot s, int16_t vpos, int16_t hpos, EventID id)
{
    assert(isEventSlot(s));
 
    // if (trace & (1 << s))
    {
        // debug("schedulePos(%d, (%d,%d), %d)\n", s, vpos, hpos, id);
    }

    Cycle cycle = amiga->dma.latchedClock;
    cycle += amiga->dma.beam2cycles(vpos, hpos);
    
    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].framePos = amiga->dma.cycle2FramePosition(cycle);
    eventSlot[s].id = id;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    if (eventSlot[s].framePos.vpos != vpos) {
        dump();
        debug("schedulePos(%d, (%d,%d), %d)\n", s, vpos, hpos, id);
        debug("f: %lld v: %d h: %d\n", eventSlot[s].framePos.frame, eventSlot[s].framePos.vpos, eventSlot[s].framePos.hpos);
    }
    assert(eventSlot[s].framePos.vpos == vpos);
    assert(eventSlot[s].framePos.hpos == hpos);
    assert(checkScheduledEvent(s));
}

void
EventHandler::rescheduleAbs(EventSlot s, Cycle cycle)
{
    assert(isEventSlot(s));
    
    if (trace & (1 << s)) {
        debug("rescheduleAbs(%d, %lld)\n", s, cycle);
    }
    
    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].framePos = amiga->dma.cycle2FramePosition(cycle);
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::rescheduleRel(EventSlot s, Cycle cycle)
{
    assert(isEventSlot(s));
    
    cycle += amiga->dma.clock;
    
    if (trace & (1 << s)) {
        debug("scheduleRel(%d, %lld)\n", s, cycle);
    }
    
    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].framePos = amiga->dma.cycle2FramePosition(cycle);
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::disable(EventSlot s)
{
    assert(isEventSlot(s));
    
    if (trace & (1 << s)) {
        debug("disable(%d)\n", s);
    }
    
    eventSlot[s].triggerCycle = INT64_MAX;
}

void
EventHandler::cancel(EventSlot s)
{
    assert(isEventSlot(s));

    if (trace & (1 << s)) {
        debug("cancel(%d)\n", s);
    }

    eventSlot[s].id = (EventID)0;
    eventSlot[s].triggerCycle = INT64_MAX;    
}

bool
EventHandler::checkScheduledEvent(EventSlot s)
{
    assert(isEventSlot(s));
    
    if (eventSlot[s].triggerCycle < 0) {
        fatalError("Scheduled event has a too small trigger cycle.");
        amiga->dma.dump();
        return false;
    }
    
    EventID id = eventSlot[s].id;
    
    if (id == 0) {
        fatalError("Event ID must not be 0.");
        return false;
    }
    
    switch (s) {
        case CIAA_SLOT:
        case CIAB_SLOT:
            if (!isCiaEvent(id)) {
                fatalError("Invalid CIA event ID.");
                return false;
            }
            if (eventSlot[s].triggerCycle != INT64_MAX && eventSlot[s].triggerCycle % 40 != 0) {
                fatalError("Scheduled trigger cycle is not a CIA cycle.");
                return false;
            }
            break;
            
        case DMA_SLOT:
            if (!isDmaEvent(id)) {
                fatalError("Invalid DMA event ID.");
                return false;
            }
            break;
            
        case COP_SLOT:
            if (!isCopEvent(id)) {
                fatalError("Invalid COP event ID.");
                return false;
            }
            break;
            
        case BLT_SLOT:
            if (!isBltEvent(id)) {
                fatalError("Invalid BLT event ID.");
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
    assert(isEventSlot(s));
    
    // Note: This function has to be called at the trigger cycle
    if (amiga->dma.clock != eventSlot[s].triggerCycle) {
        return true;
    }
    
    // Verify that the event triggers at the right beam position
    if (eventSlot[s].framePos.frame != amiga->dma.frame) {
        fatalError("Trigger frame does not match.");
        return false;
    }
    if (eventSlot[s].framePos.vpos != amiga->dma.vpos) {
        fatalError("Vertical trigger position does not match.");
        return false;
    }
    if (eventSlot[s].framePos.hpos != amiga->dma.hpos) {
        fatalError("Horizontal trigger position does not match.");
        return false;
    }

    return true;
}

void
EventHandler::_executeUntil(Cycle cycle) {
    
    // Check for a CIA A event
    if (isDue(CIAA_SLOT, cycle)) {

        assert(checkTriggeredEvent(CIAA_SLOT));

        switch(eventSlot[CIAA_SLOT].id) {
                
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
        
        switch(eventSlot[CIAB_SLOT].id) {
                
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
        amiga->dma.serviceDMAEvent(eventSlot[DMA_SLOT].id, eventSlot[DMA_SLOT].data);
    }
    
    // Check for a Copper event
    if (isDue(COP_SLOT, cycle)) {
        
        // debug("Serving COPPER event at %lld\n", cycle);
    
        assert(checkTriggeredEvent(COP_SLOT));
        amiga->dma.copper.serviceEvent(eventSlot[COP_SLOT].id, eventSlot[COP_SLOT].data);
    }
 
    // Check for a Blitter event
    if (isDue(BLT_SLOT, cycle)) {
        assert(checkTriggeredEvent(BLT_SLOT));
        amiga->dma.blitter.serviceEvent(eventSlot[BLT_SLOT].id);
    }

    // Check for a raster event
    if (isDue(RAS_SLOT, cycle)) {
        assert(checkTriggeredEvent(RAS_SLOT));
        amiga->dma.serviceRASEvent(eventSlot[RAS_SLOT].id);
    }
    
    // Check for a secondary event
    if (isDue(SEC_SLOT, cycle)) {
        _executeSecondaryUntil(cycle);
    }

    // Determine the next trigger cycle
    nextTrigger = eventSlot[0].triggerCycle;
    for (unsigned i = 1; i < EVENT_SLOT_COUNT; i++)
        if (eventSlot[i].triggerCycle < nextTrigger)
            nextTrigger = eventSlot[i].triggerCycle;
}

void
EventHandler::_executeSecondaryUntil(Cycle cycle) {

    // Check all event slots one by one
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
    nextSecTrigger = secondarySlot[0].triggerCycle;
    for (unsigned i = 1; i < SEC_SLOT_COUNT; i++)
        if (secondarySlot[i].triggerCycle < nextSecTrigger)
            nextSecTrigger = secondarySlot[i].triggerCycle;

    // Update the secondary table trigger in the primary table
    rescheduleAbs(SEC_SLOT, nextSecTrigger);
}

void
EventHandler::scheduleSecondaryAbs(EventSlot s, Cycle cycle, EventID id)
{
    assert(isSecondarySlot(s));
    
    if (trace & (1 << s))
    {
        debug("scheduleSecondaryAbs(%d, %lld, %d)\n", s, cycle, id);
    }
    
    // Schedule event in secondary table
    secondarySlot[s].triggerCycle = cycle;
    secondarySlot[s].framePos = amiga->dma.cycle2FramePosition(cycle);
    secondarySlot[s].id = id;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    if (cycle < eventSlot[SEC_SLOT].triggerCycle)
        rescheduleAbs(SEC_SLOT, cycle);
}

void
EventHandler::scheduleSecondaryRel(EventSlot s, Cycle cycle, EventID id)
{
    assert(isSecondarySlot(s));
    
    cycle += amiga->dma.clock;
    
    if (trace & (1 << s))
    {
        debug("scheduleSecondaryRel(%d, %lld, %d)\n", s, cycle, id);
    }
    
    // Schedule event in secondary table
    secondarySlot[s].triggerCycle = cycle;
    secondarySlot[s].framePos = amiga->dma.cycle2FramePosition(cycle);
    secondarySlot[s].id = id;
    if (cycle < nextSecTrigger) nextSecTrigger = cycle;
    
    // Update the secondary table trigger in the primary table
    if (cycle < eventSlot[SEC_SLOT].triggerCycle)
        rescheduleAbs(SEC_SLOT, cycle);
}

void
EventHandler::serveIRQEvent(EventSlot slot, int irqBit)
{
    if (secondarySlot[slot].id == IRQ_SET) {
        amiga->paula.setINTREQ(0x8000 | (1 << irqBit));
    } else {
        assert(secondarySlot[slot].id == IRQ_CLEAR);
        amiga->paula.setINTREQ(1 << irqBit);
    }
    secondarySlot[slot].triggerCycle = NEVER;
}
