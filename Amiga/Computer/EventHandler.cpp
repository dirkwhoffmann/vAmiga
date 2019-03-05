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
    uint32_t beam = amiga->dma.beam;
    uint16_t vp = VPOS(beam);
    uint16_t hp = HPOS(beam);
    
    plainmsg(" Master clock: %lld master cycles\n", amiga->masterClock);
    plainmsg("    DMA clock: %lld master cycles", amiga->dma.clock);
    plainmsg(" (%lld DMA cycles)\n", AS_DMA_CYCLES(amiga->dma.clock));
    plainmsg("Beam position: (%d,%d) ($%X,$%X)\n", vp, hp, vp, hp);
    plainmsg("\n");
    for (unsigned i = 0; i < EVENT_SLOT_COUNT; i++) {
        
        plainmsg("Slot %d: ", i);
        
        if (!isPending((EventSlot)i)) {
            plainmsg("no pending event\n");
            continue;
        }
            
        plainmsg("Event ID: %d  Payload: %lld  Triggers at: %lld",
                 eventSlot[i].id,
                 eventSlot[i].data,
                 eventSlot[i].triggerCycle);
        
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
EventHandler::scheduleEvent(EventSlot s, Cycle cycle, EventID id, int64_t data)
{
    assert(isEventSlot(s));
    
    // if (s == COP_SLOT)
    // debug("Scheduling event at %lld in slot %d\n", cycle, s);
    
    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].triggerBeam = INT32_MAX;
    eventSlot[s].id = id;
    eventSlot[s].data = data;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::scheduleEvent(EventSlot s, int16_t vpos, int16_t hpos, EventID id, int64_t data)
{
    Cycle cycle = amiga->dma.latchedClock;
    cycle += amiga->dma.beam2cycles(vpos, hpos);
    
    scheduleEvent(s, cycle, id, data);
}

void
EventHandler::scheduleNextEvent(EventSlot s, Cycle offset, EventID id, int64_t data)
{
    assert(isEventSlot(s));
    
    Cycle cycle = eventSlot[s].triggerCycle + offset;

    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].triggerBeam = INT32_MAX;
    eventSlot[s].id = id;
    eventSlot[s].data = data;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::scheduleEventInNextFrame(EventSlot s, int16_t vpos, int16_t hpos,
                                       EventID id, int64_t data)
{
    Cycle cycle = amiga->dma.latchedClock;
    cycle += amiga->dma.cyclesInCurrentFrame();
    cycle += amiga->dma.beam2cycles(vpos, hpos);

    scheduleEvent(s, cycle, id, data);
}

void
EventHandler::rescheduleEvent(EventSlot s, Cycle offset)
{
    assert(isEventSlot(s));
    
    Cycle cycle = eventSlot[s].triggerCycle + offset;
    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].triggerBeam = INT32_MAX;
    if (cycle < nextTrigger) nextTrigger = cycle;
    
    assert(checkScheduledEvent(s));
}

void
EventHandler::cancelEvent(EventSlot s)
{
    assert(isEventSlot(s));
    
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
        // amiga->dma.blitter.serviceEvent(eventSlot[BLT_SLOT].id, eventSlot[BLT_SLOT].data);
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
