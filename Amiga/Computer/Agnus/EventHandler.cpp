// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
Agnus::inspectEvents()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    eventInfo.masterClock = amiga->masterClock;
    eventInfo.dmaClock = clock;
    eventInfo.ciaAClock = ciaA->clock;
    eventInfo.ciaBClock  = ciaB->clock;
    eventInfo.frame = frame;
    eventInfo.vpos = vpos;
    eventInfo.hpos = hpos;

    // Inspect all slots
    for (int i = 0; i < SLOT_COUNT; i++) inspectEventSlot((EventSlot)i);
    
    pthread_mutex_unlock(&lock);
}

void
Agnus::inspectEventSlot(EventSlot nr)
{
    assert(isEventSlot(nr));
    
    EventSlotInfo *i = &eventInfo.slotInfo[nr];
    Cycle trigger = slot[nr].triggerCycle;

    i->slotName = slotName((EventSlot)nr);
    i->eventId = slot[nr].id;
    i->trigger = trigger;
    i->triggerRel = trigger - clock;
    i->currentFrame = belongsToCurrentFrame(trigger);

    if (trigger != NEVER) {

        Beam beam = cycleToBeam(trigger);
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
                case REG_BPL1MOD:   i->eventName = "REG_BPL1MOD"; break;
                case REG_BPL2MOD:   i->eventName = "REG_BPL2MOD"; break;
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
Agnus::dumpEvents()
{
    inspectEvents();
    
    amiga->dumpClock();
    
    plainmsg("Events:\n");
    for (unsigned i = 0; i < SLOT_COUNT; i++) {
        
        plainmsg("Slot: %-17s ", eventInfo.slotInfo[i].slotName);
        plainmsg("Event: %-15s ", eventInfo.slotInfo[i].eventName);
        plainmsg("Trigger: ");
        
        Cycle trigger = eventInfo.slotInfo[i].trigger;
        if (trigger == NEVER) {
            plainmsg("never\n");
        } else {
            plainmsg("%lld ", trigger);
            plainmsg("(%lld DMA cycles away)\n", AS_DMA_CYCLES(trigger - eventInfo.dmaClock));
        }
    }
}

EventInfo
Agnus::getEventInfo()
{
    EventInfo result;
    
    pthread_mutex_lock(&lock);
    result = eventInfo;
    pthread_mutex_unlock(&lock);
    
    return result;
}

EventSlotInfo
Agnus::getEventSlotInfo(int nr)
{
    assert(isEventSlot(nr));

    EventSlotInfo result;

    pthread_mutex_lock(&lock);
    result = eventInfo.slotInfo[nr];
    pthread_mutex_unlock(&lock);

    return result;
}

void
Agnus::executePrimaryEventsUntil(Cycle cycle) {

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
        serviceDMAEvent(slot[DMA_SLOT].id);
    }

    // Check for a Copper event
    if (isDue<COP_SLOT>(cycle)) {
        assert(checkTriggeredEvent(COP_SLOT));
        copper.serviceEvent(slot[COP_SLOT].id);
    }

    // Check for a Blitter event
    if (isDue<BLT_SLOT>(cycle)) {
        assert(checkTriggeredEvent(BLT_SLOT));
        blitter.serviceEvent(slot[BLT_SLOT].id);
    }

    // Check for a raster event
    if (isDue<RAS_SLOT>(cycle)) {
        // Slot is currently unused
        // assert(checkTriggeredEvent(RAS_SLOT));
    }

    // Check if a secondary event needs to be processed
    if (isDue<SEC_SLOT>(cycle)) {
        executeSecondaryEventsUntil(cycle);
    }

    // Determine the next trigger cycle
    nextTrigger = slot[0].triggerCycle;
    for (unsigned i = 1; i <= LAST_PRIM_SLOT; i++)
        if (slot[i].triggerCycle < nextTrigger)
            nextTrigger = slot[i].triggerCycle;
}

void
Agnus::executeSecondaryEventsUntil(Cycle cycle) {

    // Check all secondary event slots one by one
    if (isDue<DSK_SLOT>(cycle)) {
        paula->diskController.serveDiskEvent();
    }
    if (isDue<IRQ_TBE_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_TBE_SLOT, 0);
    }
    if (isDue<IRQ_DSKBLK_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_DSKBLK_SLOT, 1);
    }
    if (isDue<IRQ_SOFT_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_SOFT_SLOT, 2);
    }
    if (isDue<IRQ_PORTS_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_PORTS_SLOT, 3);
    }
    if (isDue<IRQ_COPR_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_COPR_SLOT, 4);
    }
    if (isDue<IRQ_VERTB_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_VERTB_SLOT, 5);
    }
    if (isDue<IRQ_BLIT_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_BLIT_SLOT, 6);
    }
    if (isDue<IRQ_AUD0_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_AUD0_SLOT, 7);
    }
    if (isDue<IRQ_AUD1_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_AUD1_SLOT, 8);
    }
    if (isDue<IRQ_AUD2_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_AUD2_SLOT, 9);
    }
    if (isDue<IRQ_AUD3_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_AUD3_SLOT, 10);
    }
    if (isDue<IRQ_RBF_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_RBF_SLOT, 11);
    }
    if (isDue<IRQ_DSKSYN_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_DSKSYN_SLOT, 12);
    }
    if (isDue<IRQ_EXTER_SLOT>(cycle)) {
        serviceIRQEvent(IRQ_EXTER_SLOT, 13);
    }
    if (isDue<REG_COP_SLOT>(cycle)) {
        serviceREGEvent(REG_COP_SLOT);
    }
    if (isDue<REG_CPU_SLOT>(cycle)) {
        serviceREGEvent(REG_CPU_SLOT);
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
        serviceSYNCEvent(slot[SYNC_SLOT].id);
    }
    if (isDue<INSPECTOR_SLOT>(cycle)) {
        serveINSEvent();
    }

    // Determine the next trigger cycle
    Cycle nextSecTrigger = slot[FIRST_SEC_SLOT].triggerCycle;
    for (unsigned i = FIRST_SEC_SLOT + 1; i <= LAST_SEC_SLOT; i++)
        if (slot[i].triggerCycle < nextSecTrigger)
            nextSecTrigger = slot[i].triggerCycle;

    // Update the secondary table trigger in the primary table
    rescheduleAbs<SEC_SLOT>(nextSecTrigger);
}

void
Agnus::serviceDMAEvent(EventID id)
{
    switch (id) {

        case DMA_DISK:

            if (paula->diskController.getFifoBuffering())
                paula->diskController.performDMA();
            else
                paula->diskController.performSimpleDMA();
            break;

        case DMA_A0:
            break;

        case DMA_A1:
            break;

        case DMA_A2:
            break;

        case DMA_A3:
            break;

        case DMA_S0_1:
            executeFirstSpriteCycle<0>();
            break;

        case DMA_S1_1:
            executeFirstSpriteCycle<1>();
            break;

        case DMA_S2_1:
            executeFirstSpriteCycle<2>();
            break;

        case DMA_S3_1:
            executeFirstSpriteCycle<3>();
            break;

        case DMA_S4_1:
            executeFirstSpriteCycle<4>();
            break;

        case DMA_S5_1:
            executeFirstSpriteCycle<5>();
            break;

        case DMA_S6_1:
            executeFirstSpriteCycle<6>();
            break;

        case DMA_S7_1:
            executeFirstSpriteCycle<7>();
            break;

        case DMA_S0_2:
            executeSecondSpriteCycle<0>();
            break;

        case DMA_S1_2:
            executeSecondSpriteCycle<1>();
            break;

        case DMA_S2_2:
            executeSecondSpriteCycle<2>();
            break;

        case DMA_S3_2:
            executeSecondSpriteCycle<3>();
            break;

        case DMA_S4_2:
            executeSecondSpriteCycle<4>();
            break;

        case DMA_S5_2:
            executeSecondSpriteCycle<5>();
            break;

        case DMA_S6_2:
            executeSecondSpriteCycle<6>();
            break;

        case DMA_S7_2:
            executeSecondSpriteCycle<7>();
            break;

        case DMA_H1_FIRST:
            denise->prepareShiftRegisters();
            // fallthrough

        case DMA_H1:
            assert(!isLastHx(hpos));
            denise->bpldat[PLANE1] = doBitplaneDMA<0>();
            denise->fillShiftRegisters();
            denise->drawHires(16);
            break;

        case DMA_H1_LAST:
            assert(isLastHx(hpos));
            denise->bpldat[PLANE1] = doBitplaneDMA<0>();
            denise->fillShiftRegisters();
            denise->drawHires(16 + denise->scrollHiresOdd);
            if(isLastHx(hpos)) addBPLMOD<0>();
            // addBPLxMOD();
            break;

        case DMA_L1_FIRST:
            denise->prepareShiftRegisters();
            // fallthrough

        case DMA_L1:
            assert(!isLastLx(hpos));
            denise->bpldat[PLANE1] = doBitplaneDMA<0>();
            denise->fillShiftRegisters();
            denise->drawLores(16);
            break;

        case DMA_L1_LAST:
            assert(isLastLx(hpos));
            denise->bpldat[PLANE1] = doBitplaneDMA<0>();
            denise->fillShiftRegisters();
            denise->drawLores(16 + denise->scrollHiresOdd);
            if(isLastLx(hpos)) addBPLMOD<0>();
            // addBPLxMOD();
            break;

        case DMA_H2:
            denise->bpldat[PLANE2] = doBitplaneDMA<1>();
            if(isLastHx(hpos)) addBPLMOD<1>();
            break;

        case DMA_L2:
            denise->bpldat[PLANE2] = doBitplaneDMA<1>();
            if(isLastLx(hpos)) addBPLMOD<1>();
            break;

        case DMA_H3:
            denise->bpldat[PLANE3] = doBitplaneDMA<2>();
            if(isLastHx(hpos)) addBPLMOD<2>();
            break;

        case DMA_L3:
            denise->bpldat[PLANE3] = doBitplaneDMA<2>();
            if(isLastLx(hpos)) addBPLMOD<2>();
            break;

        case DMA_H4:
            denise->bpldat[PLANE4] = doBitplaneDMA<3>();
            if(isLastHx(hpos)) addBPLMOD<3>();
            break;

        case DMA_L4:
            denise->bpldat[PLANE4] = doBitplaneDMA<3>();
            if(isLastLx(hpos)) addBPLMOD<3>();
            break;

        case DMA_L5:
            denise->bpldat[PLANE5] = doBitplaneDMA<4>();
            if(isLastLx(hpos)) addBPLMOD<4>();
            break;

        case DMA_L6:
            denise->bpldat[PLANE6] = doBitplaneDMA<5>();
            if(isLastLx(hpos)) addBPLMOD<5>();
            break;

        default:
            debug("id = %d\n", id);
            assert(false);
    }

    // Schedule next event
    uint8_t next = nextDmaEvent[hpos];
    // debug("id = %d hpos = %d, next = %d\n", id, hpos, next);
    if (next) {
        // events.schedulePos<DMA_SLOT>(vpos, next, dmaEvent[next]);
        scheduleRel<DMA_SLOT>(DMA_CYCLES(next - hpos), dmaEvent[next]);
    } else {
        cancel<DMA_SLOT>();
    }
}

void
Agnus::serviceIRQEvent(EventSlot s, int irqBit)
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
    
    cancel(s);
}

void
Agnus::serviceREGEvent(EventSlot nr)
{
    EventID id = slot[nr].id;
    uint16_t data = (uint16_t)slot[nr].data;

    // debug("serveRegEvent(%d)\n", slot);

    switch (id) {

        case REG_DIWSTRT: setDIWSTRT((uint16_t)data); break;
        case REG_DIWSTOP: setDIWSTOP((uint16_t)data); break;
        case REG_BPL1MOD: setBPL1MOD((uint16_t)data); break;
        case REG_BPL2MOD: setBPL2MOD((uint16_t)data); break;
        default: assert(false);
    }

    // Remove event
    cancel(nr);
}

void
Agnus::serviceSYNCEvent(EventID id)
{
    switch (id) {

        case SYNC_H:

            hsyncHandler();
            break;

        default:
            assert(false);
            break;
    }
}

void
Agnus::serveINSEvent()
{
    switch (slot[INSPECTOR_SLOT].id) {

        case INS_NONE:   break;
        case INS_AMIGA:  amiga->inspect(); break;
        case INS_CPU:    amiga->cpu.inspect(); break;
        case INS_MEM:    mem->inspect(); break;
        case INS_CIA:    ciaA->inspect(); ciaB->inspect(); break;
        case INS_AGNUS:  inspect(); break;
        case INS_PAULA:  paula->inspect(); break;
        case INS_DENISE: denise->inspect(); break;
        case INS_PORTS:
            amiga->serialPort.inspect();
            amiga->paula.uart.inspect();
            amiga->controlPort1.inspect();
            amiga->controlPort2.inspect();
            break;
        case INS_EVENTS: inspectEvents(); break;
        default:         assert(false);
    }

    // Reschedule event
    rescheduleRel<INSPECTOR_SLOT>((Cycle)(inspectionInterval * 28000000));
}

void
Agnus::scheduleRegEvent(EventSlot slot, Cycle cycle, EventID id, int64_t data)
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
                serviceREGEvent(slot);
            }
            scheduleRel<REG_COP_SLOT>(cycle, id, data);
            break;

        case REG_CPU_SLOT:
            if (hasEvent<REG_CPU_SLOT>()) {
                assert(isDue<REG_CPU_SLOT>(amiga->masterClock));
                serviceREGEvent(slot);
            }
            scheduleRel<REG_CPU_SLOT>(cycle, id, data);
            break;

        default:
            assert(false);
    }
}

bool
Agnus::checkScheduledEvent(EventSlot s)
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
Agnus::checkTriggeredEvent(EventSlot s)
{
    assert(isPrimarySlot(s));
    
    // Note: This function has to be called at the trigger cycle
    if (clock != slot[s].triggerCycle) {
        return true;
    }
    
    return true;
}
