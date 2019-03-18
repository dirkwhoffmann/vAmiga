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
    amiga->dumpClock();
    _dumpPrimaryTable();
    _dumpSecondaryTable();
}

void
EventHandler::_dumpPrimaryTable()
{
    const char *slotName;
    const char *eventName;
    
    plainmsg("\nPrimary event table:\n");
    for (unsigned i = 0; i < EVENT_SLOT_COUNT; i++) {
        
        switch ((EventSlot)i) {
                
            case CIAA_SLOT:
            case CIAB_SLOT:
                
                slotName = (i == 0) ? "CIA A" : "CIA B";
                switch (eventSlot[i].id) {
                    case 0:                eventName = "none"; break;
                    case CIA_EXECUTE:      eventName = "CIA_EXECUTE"; break;
                    case CIA_WAKEUP:       eventName = "CIA_WAKEUP"; break;
                    default:               eventName = "*** INVALID ***"; break;
                }
                break;
                
            case DMA_SLOT:
                
                slotName = "DMA";
                switch (eventSlot[i].id) {
                    case 0:                eventName = "none"; break;
                    case DMA_DISK:         eventName = "DMA_DISK"; break;
                    case DMA_A0:           eventName = "A0"; break;
                    case DMA_A1:           eventName = "A1"; break;
                    case DMA_A2:           eventName = "A2"; break;
                    case DMA_A3:           eventName = "A3"; break;
                    case DMA_S0:           eventName = "S0"; break;
                    case DMA_S1:           eventName = "S1"; break;
                    case DMA_S2:           eventName = "S2"; break;
                    case DMA_S3:           eventName = "S3"; break;
                    case DMA_S4:           eventName = "S4"; break;
                    case DMA_S5:           eventName = "S5"; break;
                    case DMA_S6:           eventName = "S6"; break;
                    case DMA_S7:           eventName = "S7"; break;
                    case DMA_L1:           eventName = "L1"; break;
                    case DMA_L2:           eventName = "L2"; break;
                    case DMA_L3:           eventName = "L3"; break;
                    case DMA_L4:           eventName = "L4"; break;
                    case DMA_L5:           eventName = "L5"; break;
                    case DMA_L6:           eventName = "L6"; break;
                    case DMA_H1:           eventName = "H1"; break;
                    case DMA_H2:           eventName = "H2"; break;
                    case DMA_H3:           eventName = "H3"; break;
                    case DMA_H4:           eventName = "H4"; break;
                    default:               eventName = "*** INVALID ***"; break;
                }
                break;
                
            case COP_SLOT:
                
                slotName = "Copper";
                switch (eventSlot[i].id) {
                        
                    case 0:                eventName = "none"; break;
                    case COP_REQUEST_DMA:  eventName = "COP_REQUEST_DMA"; break;
                    case COP_FETCH:        eventName = "COP_FETCH"; break;
                    case COP_MOVE:         eventName = "COP_MOVE"; break;
                    case COP_WAIT_OR_SKIP: eventName = "WAIT_OR_SKIP"; break;
                    case COP_WAIT:         eventName = "COP_WAIT"; break;
                    case COP_SKIP:         eventName = "COP_SKIP"; break;
                    case COP_JMP1:         eventName = "COP_JMP1"; break;
                    case COP_JMP2:         eventName = "COP_JMP2"; break;
                    default:               eventName = "*** INVALID ***"; break;
                }
                break;
                
            case BLT_SLOT:
                
                slotName = "Blitter";
                switch (eventSlot[i].id) {
                        
                    case 0:                eventName = "none"; break;
                    case BLT_INIT:         eventName = "BLT_INIT"; break;
                    case BLT_EXECUTE:      eventName = "BLT_EXECUTE"; break;
                    default:               eventName = "*** INVALID ***"; break;
                }
                break;
            
            case RAS_SLOT:
                
                slotName = "Raster";
                switch (eventSlot[i].id) {
                        
                    case 0:                eventName = "none"; break;
                    case RAS_HSYNC:        eventName = "RAS_HSYNC"; break;
                    case RAS_DIWSTRT:      eventName = "RAS_DIWSTRT"; break;
                    case RAS_DIWDRAW:      eventName = "RAS_DIWDRAW"; break;
                    default:               eventName = "*** INVALID ***"; break;
                }
                break;
    
            case SEC_SLOT:
                
                slotName = "Secondary";
                switch (eventSlot[i].id) {
                        
                    case 0:                eventName = "none"; break;
                    case SEC_TRIGGER:      eventName = "SEC_TRIGGER"; break;
                    default:               eventName = "*** INVALID ***"; break;
                }
                break;
                
            default: assert(false);
        }
        
        _dumpSlot(slotName, eventName, eventSlot[i]);
    }
}

void
EventHandler::_dumpSecondaryTable()
{
    const char *slotName;
    const char *eventName;
    
    plainmsg("\nSecondary event table:\n");
    for (unsigned i = 0; i < SEC_SLOT_COUNT; i++) {
        
        switch ((EventSlot)i) {
                
            case HSYNC_SLOT:         slotName = "HSYNC"; break;
            case TBE_IRQ_SLOT:       slotName = "TBE_IRQ"; break;
            case DSKBLK_IRQ_SLOT:    slotName = "DSKBLK_IRQ"; break;
            case SOFT_IRQ_SLOT:      slotName = "SOFT_IRQ"; break;
            case PORTS_IRQ_SLOT:     slotName = "PORTS_IRQ"; break;
            case COPR_IRQ_SLOT:      slotName = "COPR_IRQ"; break;
            case VERTB_IRQ_SLOT:     slotName = "VERTB_IRQ"; break;
            case BLIT_IRQ_SLOT:      slotName = "BLIT_IRQ"; break;
            case AUD0_IRQ_SLOT:      slotName = "AUD0_IRQ"; break;
            case AUD1_IRQ_SLOT:      slotName = "AUD1_IRQ"; break;
            case AUD2_IRQ_SLOT:      slotName = "AUD2_IRQ"; break;
            case AUD3_IRQ_SLOT:      slotName = "AUD3_IRQ"; break;
            case RBF_IRQ_SLOT:       slotName = "RBF_IRQ"; break;
            case DSKSYN_IRQ_SLOT:    slotName = "DSKSYN_IRQ"; break;
            case EXTER_IRQ_SLOT:     slotName = "EXTER_IRQ"; break;
            default:                 slotName = "*** INVALID ***"; break;
        }
        
        switch ((EventSlot)i) {
                
            case HSYNC_SLOT:
                
                switch (secondarySlot[i].id) {
                        
                    case 0:          eventName = "none"; break;
                    case HSYNC_EOL:  eventName = "HSYNC_EOL"; break;
                    default:         eventName = "*** INVALID ***"; break;
                }
                break;
                
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
                        
                    case 0:          eventName = "none"; break;
                    case IRQ_SET:    eventName = "IRQ_SET"; break;
                    case IRQ_CLEAR:  eventName = "IRQ_CLEAR"; break;
                    default:         eventName = "*** INVALID ***"; break;
                }
                break;
                
            default: assert(false);
        }
        
        _dumpSlot(slotName, eventName, secondarySlot[i]);

    }
}

void
EventHandler::_dumpSlot(const char *slotName, const char *eventName, const Event event)
{
    plainmsg("Slot: %-10s ", slotName);
    plainmsg("Event: %-15s ", eventName);
    plainmsg("Trigger: ");
    
    Cycle trigger = event.triggerCycle;
    if (trigger == NEVER) {
        plainmsg("never");
    } else {
        plainmsg("%lld (%lld,%d,%d) ",
                 trigger,
                 event.framePos.frame,
                 event.framePos.vpos,
                 event.framePos.hpos);
        plainmsg("(%lld DMA cycles away)\n",
                 AS_DMA_CYCLES(trigger - amiga->dma.clock));

    }
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
    if (isDueSec(HSYNC_SLOT, cycle)) { // DEPRECATED
        secondarySlot[HSYNC_SLOT].triggerCycle = NEVER;
        assert(false);
    }
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
