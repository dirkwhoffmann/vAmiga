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
    trace = (1 << BLT_SLOT);
    
    for (unsigned i = 0; i < EVENT_SLOT_COUNT; i++) {
        eventSlot[i].triggerCycle = INT64_MAX;
        eventSlot[i].triggerBeam = INT32_MAX;
        eventSlot[i].id = (EventID)0;
        eventSlot[i].data = 0;
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
    const char *slotName;
    const char *eventName;
    
    amiga->dumpClock();
    for (unsigned i = 0; i < EVENT_SLOT_COUNT; i++) {
        
        switch ((EventSlot)i) {
                
            case CIAA_SLOT:
            case CIAB_SLOT:
                
                slotName = (i == 0) ? "CIA A" : "CIA B";
                switch (eventSlot[i].id) {
                    case 0:                eventName = "none           "; break;
                    case CIA_EXECUTE:      eventName = "CIA_EXECUTE    "; break;
                    case CIA_WAKEUP:       eventName = "CIA_WAKEUP     "; break;
                    default:               eventName = "*** INVALID ***"; break;
                }
                break;
                
            case DMA_SLOT:
                
                slotName = "DMA";
                switch (eventSlot[i].id) {
                    case 0:                eventName = "none           "; break;
                    case DMA_DISK:         eventName = "DMA_DISK       "; break;
                    case DMA_A0:           eventName = "A0             "; break;
                    case DMA_A1:           eventName = "A1             "; break;
                    case DMA_A2:           eventName = "A2             "; break;
                    case DMA_A3:           eventName = "A3             "; break;
                    case DMA_S0:           eventName = "S0             "; break;
                    case DMA_S1:           eventName = "S1             "; break;
                    case DMA_S2:           eventName = "S2             "; break;
                    case DMA_S3:           eventName = "S3             "; break;
                    case DMA_S4:           eventName = "S4             "; break;
                    case DMA_S5:           eventName = "S5             "; break;
                    case DMA_S6:           eventName = "S6             "; break;
                    case DMA_S7:           eventName = "S7             "; break;
                    case DMA_L1:           eventName = "L1             "; break;
                    case DMA_L2:           eventName = "L2             "; break;
                    case DMA_L3:           eventName = "L3             "; break;
                    case DMA_L4:           eventName = "L4             "; break;
                    case DMA_L5:           eventName = "L5             "; break;
                    case DMA_L6:           eventName = "L6             "; break;
                    case DMA_H1:           eventName = "H1             "; break;
                    case DMA_H2:           eventName = "H2             "; break;
                    case DMA_H3:           eventName = "H3             "; break;
                    case DMA_H4:           eventName = "H4             "; break;
                    default:               eventName = "*** INVALID ***"; break;
                }
                break;
                
            case COP_SLOT:
                
                slotName = "Copper";
                switch (eventSlot[i].id) {
                        
                    case 0:                eventName = "none           "; break;
                    case COP_REQUEST_DMA:  eventName = "COP_REQUEST_DMA"; break;
                    case COP_FETCH:        eventName = "COP_FETCH      "; break;
                    case COP_MOVE:         eventName = "COP_MOVE       "; break;
                    case COP_WAIT_OR_SKIP: eventName = "WAIT_OR_SKIP   "; break;
                    case COP_WAIT:         eventName = "COP_WAIT       "; break;
                    case COP_SKIP:         eventName = "COP_SKIP       "; break;
                    case COP_JMP1:         eventName = "COP_JMP1       "; break;
                    case COP_JMP2:         eventName = "COP_JMP2       "; break;
                    default:               eventName = "*** INVALID ***"; break;
                }
                break;
                
            case BLT_SLOT:
                
                slotName = "Blitter";
                switch (eventSlot[i].id) {
                        
                    case 0:                eventName = "none           "; break;
                    case BLT_INIT:         eventName = "BLT_INIT       "; break;
                    case BLT_EXECUTE:      eventName = "BLT_EXECUTE    "; break;
                    default:               eventName = "*** INVALID ***"; break;
                }
                break;
                
            case RAS_SLOT:
                
                slotName = "Raster";
                switch (eventSlot[i].id) {
                        
                    case 0:                eventName = "none           "; break;
                    case RAS_HSYNC:        eventName = "RAS_HSYNC      "; break;
                    default:               eventName = "*** INVALID ***"; break;
                }
                break;
    
            default: assert(false);
        }
        
        plainmsg("%7s: ", slotName);
        plainmsg("Event: %s ", eventName);
        plainmsg("Trigger: ");

        Cycle trigger = eventSlot[i].triggerCycle;
        if (trigger == INT64_MAX) {
            plainmsg("disabled");
        } else {
            plainmsg("%lld (%lld DMA cycles away)",
                     trigger, AS_DMA_CYCLES(trigger - amiga->dma.clock));
        }
        
        if (eventSlot[i].triggerBeam != INT32_MAX) {
            plainmsg(" (%d,%d)\n",
                     VPOS(eventSlot[i].triggerBeam),
                     HPOS(eventSlot[i].triggerBeam));
        } else {
            plainmsg("\n");
        }
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
    eventSlot[s].triggerBeam = INT32_MAX;
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
    eventSlot[s].triggerBeam = INT32_MAX;
    eventSlot[s].id = id;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::schedulePos(EventSlot s, int16_t vpos, int16_t hpos, EventID id)
{
    assert(isEventSlot(s));
 
    if (trace & (1 << s)) {
        debug("schedulePos(%d, (%d,%d), %d)\n", s, vpos, hpos, id);
    }

    Cycle cycle = amiga->dma.latchedClock;
    cycle += amiga->dma.beam2cycles(vpos, hpos);
    
    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].triggerBeam = INT32_MAX;
    eventSlot[s].id = id;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::reschedule(EventSlot s, Cycle offset)
{
    assert(isEventSlot(s));
    
    if (trace & (1 << s)) {
        debug("reschedule(%d, %lld)\n", s, offset);
    }

    Cycle cycle = eventSlot[s].triggerCycle + offset;
    
    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].triggerBeam = INT32_MAX;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::reschedule(EventSlot s, Cycle offset, EventID id)
{
    reschedule(s, offset);
    eventSlot[s].id = id;
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
    
    if (eventSlot[s].triggerCycle <= amiga->dma.clock) {
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
            if (eventSlot[s].id == 0 || eventSlot[s].id >= CIA_EVENT_COUNT) {
                fatalError("Invalid CIA event ID.");
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
    
    // Verify that the event triggers at the right beam position
    uint32_t beam = eventSlot[s].triggerBeam;
    if (beam != INT32_MAX && beam != amiga->dma.beam) {
        fatalError("Trigger beam position does not match.");
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
        amiga->dma.hsyncHandler();
    }
    
    // Determine the next trigger cycle
    nextTrigger = eventSlot[0].triggerCycle;
    for (unsigned i = 1; i < EVENT_SLOT_COUNT; i++)
        if (eventSlot[i].triggerCycle < nextTrigger)
            nextTrigger = eventSlot[i].triggerCycle;
}
