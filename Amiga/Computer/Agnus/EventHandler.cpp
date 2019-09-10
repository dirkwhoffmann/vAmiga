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
    
    eventInfo.masterClock = amiga->getMasterClock();
    eventInfo.dmaClock = clock;
    eventInfo.ciaAClock = ciaA->clock;
    eventInfo.ciaBClock  = ciaB->clock;
    eventInfo.frame = frame;
    eventInfo.vpos = pos.v;
    eventInfo.hpos = pos.h;

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

    if (belongsToCurrentFrame(trigger)) {
        Beam beam = cycleToBeam(trigger);
        i->vpos = beam.v;
        i->hpos = beam.h;
        i->frameRel = 0;
    } else if (belongsToNextFrame(trigger)) {
        i->vpos = 0;
        i->hpos = 0;
        i->frameRel = 1;
    } else {
        assert(belongsToPreviousFrame(trigger));
        i->vpos = 0;
        i->hpos = 0;
        i->frameRel = -1;
    }

    switch ((EventSlot)nr) {

        case REG_SLOT:
            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case REG_CHANGE:    i->eventName = "REG_CHANGE"; break;
                case REG_HSYNC:     i->eventName = "REG_HSYNC"; break;
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

        case AGN_SLOT:
            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case AGN_ACTIONS:   i->eventName = "AGN_ACTIONS"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case CIAA_SLOT:
        case CIAB_SLOT:

            switch (slot[nr].id) {
                case 0:             i->eventName = "none"; break;
                case CIA_EXECUTE:   i->eventName = "CIA_EXECUTE"; break;
                case CIA_WAKEUP:    i->eventName = "CIA_WAKEUP"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case BPL_SLOT:

            switch (slot[nr].id) {
                case 0:             i->eventName = "none"; break;
                case BPL_L1:        i->eventName = "BPL_L1"; break;
                case BPL_L2:        i->eventName = "BPL_L2"; break;
                case BPL_L3:        i->eventName = "BPL_L3"; break;
                case BPL_L4:        i->eventName = "BPL_L4"; break;
                case BPL_L5:        i->eventName = "BPL_L5"; break;
                case BPL_L6:        i->eventName = "BPL_L6"; break;
                case BPL_H1:        i->eventName = "BPL_H1"; break;
                case BPL_H2:        i->eventName = "BPL_H2"; break;
                case BPL_H3:        i->eventName = "BPL_H3"; break;
                case BPL_H4:        i->eventName = "BPL_H4"; break;
                case BPL_EOL:       i->eventName = "BPL_EOL"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case DAS_SLOT:

            switch (slot[nr].id) {
                case 0:             i->eventName = "none"; break;
                case DAS_REFRESH:   i->eventName = "DAS_REFRESH"; break;
                case DAS_D0:        i->eventName = "DAS_D0"; break;
                case DAS_D1:        i->eventName = "DAS_D1"; break;
                case DAS_D2:        i->eventName = "DAS_D2"; break;
                case DAS_A0:        i->eventName = "DAS_A0"; break;
                case DAS_A1:        i->eventName = "DAS_A1"; break;
                case DAS_A2:        i->eventName = "DAS_A2"; break;
                case DAS_A3:        i->eventName = "DAS_A3"; break;
                case DAS_S0_1:      i->eventName = "DAS_S0_1"; break;
                case DAS_S0_2:      i->eventName = "DAS_S0_2"; break;
                case DAS_S1_1:      i->eventName = "DAS_S1_1"; break;
                case DAS_S1_2:      i->eventName = "DAS_S1_2"; break;
                case DAS_S2_1:      i->eventName = "DAS_S2_2"; break;
                case DAS_S3_1:      i->eventName = "DAS_S3_1"; break;
                case DAS_S3_2:      i->eventName = "DAS_S3_2"; break;
                case DAS_S4_1:      i->eventName = "DAS_S4_1"; break;
                case DAS_S4_2:      i->eventName = "DAS_S4_2"; break;
                case DAS_S5_1:      i->eventName = "DAS_S5_1"; break;
                case DAS_S5_2:      i->eventName = "DAS_S5_2"; break;
                case DAS_S6_1:      i->eventName = "DAS_S6_1"; break;
                case DAS_S6_2:      i->eventName = "DAS_S6_2"; break;
                case DAS_S7_1:      i->eventName = "DAS_S7_1"; break;
                case DAS_S7_2:      i->eventName = "DAS_S7_2"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case COP_SLOT:

            switch (slot[nr].id) {

                case 0:                i->eventName = "none"; break;
                case COP_REQ_DMA:      i->eventName = "COP_REQ_DMA"; break;
                case COP_FETCH:        i->eventName = "COP_FETCH"; break;
                case COP_MOVE:         i->eventName = "COP_MOVE"; break;
                case COP_WAIT_OR_SKIP: i->eventName = "WAIT_OR_SKIP"; break;
                case COP_WAIT1:        i->eventName = "COP_WAIT1"; break;
                case COP_WAIT2:        i->eventName = "COP_WAIT2"; break;
                case COP_SKIP1:        i->eventName = "COP_SKIP1"; break;
                case COP_SKIP2:        i->eventName = "COP_SKIP1"; break;
                case COP_JMP1:         i->eventName = "COP_JMP1"; break;
                case COP_JMP2:         i->eventName = "COP_JMP2"; break;
                default:               i->eventName = "*** INVALID ***"; break;
            }
            break;

        case BLT_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case BLT_START:     i->eventName = "BLT_START"; break;
                case BLT_EXEC_SLOW: i->eventName = "BLT_EXEC_SLOW"; break;
                case BLT_EXEC_FAST: i->eventName = "BLT_EXEC_FAST"; break;
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

        case DCH_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case DCH_INSERT:    i->eventName = "DCH_INSERT"; break;
                case DCH_EJECT:     i->eventName = "DCH_EJECT"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case IRQ_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case IRQ_CHECK:     i->eventName = "IRQ_CHECK"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case KBD_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case KBD_SELFTEST:  i->eventName = "KBD_SELFTEST"; break;
                case KBD_SYNC:      i->eventName = "KBD_SYNC"; break;
                case KBD_STRM_ON:   i->eventName = "KBD_STRM_ON"; break;
                case KBD_STRM_OFF:  i->eventName = "KBD_STRM_OFF"; break;
                case KBD_TIMEOUT:   i->eventName = "KBD_TIMEOUT"; break;
                case KBD_SEND:      i->eventName = "KBD_SEND"; break;
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

        case INS_SLOT:

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
Agnus::scheduleNextBplEvent(int16_t hpos)
{
    assert(isHPos(hpos));

    uint8_t next = nextDmaEvent[hpos];
    scheduleRel<BPL_SLOT>(DMA_CYCLES(next - pos.h), dmaEvent[next]);

    assert(hasEvent<BPL_SLOT>());
}

void
Agnus::scheduleBplEventForCycle(int16_t hpos)
{
    assert(isHPos(hpos));
    assert(hpos >= pos.h);

    if (dmaEvent[hpos] != EVENT_NONE) {
        scheduleRel<BPL_SLOT>(DMA_CYCLES(hpos - pos.h), dmaEvent[hpos]);
    } else {
        scheduleNextBplEvent(hpos);
    }

    assert(hasEvent<BPL_SLOT>());
}

void
Agnus::scheduleNextREGEvent()
{
    // Determine when the next register change happens
    Cycle nextTrigger = changeRecorder.trigger();

    // Schedule a register change event for that cycle
    scheduleAbs<REG_SLOT>(nextTrigger, REG_CHANGE);
}

void
Agnus::executeEventsUntil(Cycle cycle) {

    //
    // Check primary slots
    //

    if (isDue<REG_SLOT>(cycle)) {
        serviceREGEvent(cycle);
    }
    if (isDue<RAS_SLOT>(cycle)) {
        serviceRASEvent();
    }
    if (isDue<AGN_SLOT>(cycle)) {
        serviceAGNEvent();
    }
    if (isDue<CIAA_SLOT>(cycle)) {
        serviceCIAEvent<0>();
    }
    if (isDue<CIAB_SLOT>(cycle)) {
        serviceCIAEvent<1>();
    }
    if (isDue<BPL_SLOT>(cycle)) {
        serviceBPLEvent();
    }
    if (isDue<DAS_SLOT>(cycle)) {
        serviceDASEvent();
    }
    if (isDue<COP_SLOT>(cycle)) {
        copper.serviceEvent(slot[COP_SLOT].id);
    }
    if (isDue<BLT_SLOT>(cycle)) {
        blitter.serviceEvent(slot[BLT_SLOT].id);
    }

    if (isDue<SEC_SLOT>(cycle)) {

        //
        // Check secondary slots
        //

        if (isDue<DSK_SLOT>(cycle)) {
            paula->diskController.serviceDiskEvent();
        }
        if (isDue<DCH_SLOT>(cycle)) {
            paula->diskController.serviceDiskChangeEvent(slot[DCH_SLOT].id, (int)slot[DCH_SLOT].data);
        }
        if (isDue<IRQ_SLOT>(cycle)) {
            paula->serviceIrqEvent();
        }
        if (isDue<KBD_SLOT>(cycle)) {
            amiga->keyboard.serviceKeyboardEvent(slot[KBD_SLOT].id);
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
        if (isDue<INS_SLOT>(cycle)) {
            serviceINSEvent();
        }

        // Determine the next trigger cycle for all secondary slots
        Cycle nextSecTrigger = slot[SEC_SLOT + 1].triggerCycle;
        for (unsigned i = SEC_SLOT + 2; i < SLOT_COUNT; i++)
            if (slot[i].triggerCycle < nextSecTrigger)
                nextSecTrigger = slot[i].triggerCycle;

        // Update the secondary table trigger in the primary table
        rescheduleAbs<SEC_SLOT>(nextSecTrigger);
    }

    // Determine the next trigger cycle for all primary slots
    nextTrigger = slot[0].triggerCycle;
    for (unsigned i = 1; i <= SEC_SLOT; i++)
        if (slot[i].triggerCycle < nextTrigger)
            nextTrigger = slot[i].triggerCycle;
}

template <int nr> void
Agnus::serviceCIAEvent()
{
    EventSlot slotNr = (nr == 0) ? CIAA_SLOT : CIAB_SLOT;

    assert(checkTriggeredEvent(slotNr));

    switch(slot[slotNr].id) {

        case CIA_EXECUTE:
            nr ? ciaB->executeOneCycle() : ciaA->executeOneCycle();
            break;

        case CIA_WAKEUP:
            nr ? ciaB->wakeUp() : ciaA->wakeUp();
            break;

        default:
            assert(false);
    }
}

void
Agnus::serviceREGEvent(Cycle until)
{
    assert(checkTriggeredEvent(REG_SLOT));

    // Iterate through all recorded register changes
    while (!changeRecorder.isEmpty()) {

        // We're done once the trigger cycle exceeds the target cycle
        if (changeRecorder.trigger() > until) return;

        // Apply the register change
        uint32_t addr = changeRecorder.addr();
        uint16_t value = changeRecorder.value();

#ifdef USE_REG_CHANGE_SLOT

        switch (addr) {

            case REG_BPLCON0_AGNUS: setBPLCON0(bplcon0, value); break;
            case REG_BPLCON0_DENISE: denise->setBPLCON0(denise->bplcon0, value); break;
            case REG_BPLCON1: denise->setBPLCON1(value); break;
            case REG_BPLCON2: denise->setBPLCON2(value); break;
            case REG_DMACON: setDMACON(dmacon, value); break;
            case REG_DIWSTRT: setDIWSTRT(value); break;
            case REG_DIWSTOP: setDIWSTOP(value); break;
            case REG_BPL1MOD: setBPL1MOD(value); break;
            case REG_BPL2MOD: setBPL2MOD(value); break;
            case REG_BPL1PTH: setBPLxPTH(1, value); break;
            case REG_BPL1PTL: setBPLxPTL(1, value); break;
            case REG_BPL2PTH: setBPLxPTH(2, value); break;
            case REG_BPL2PTL: setBPLxPTL(2, value); break;
            case REG_BPL3PTH: setBPLxPTH(3, value); break;
            case REG_BPL3PTL: setBPLxPTL(3, value); break;
            case REG_BPL4PTH: setBPLxPTH(4, value); break;
            case REG_BPL4PTL: setBPLxPTL(4, value); break;
            case REG_BPL5PTH: setBPLxPTH(5, value); break;
            case REG_BPL5PTL: setBPLxPTL(5, value); break;
            case REG_BPL6PTH: setBPLxPTH(6, value); break;
            case REG_BPL6PTL: setBPLxPTL(6, value); break;

            default:
                warn("Register change ID %d is invalid.\n", addr);
                assert(false);
        }

#endif

        changeRecorder.remove();
    }

    // Schedule the next register change event
    scheduleNextREGEvent();
}

void
Agnus::serviceAGNEvent()
{
    assert(checkTriggeredEvent(AGN_SLOT));

    // The event should only fire if at least one action flag is set
    assert(actions);

    // Check for horizontal sync
    // if (actions & AGN_HSYNC) hsyncHandlerOld();

    // Handle all pending register changes
#ifndef USE_REG_CHANGE_SLOT
    if (actions & AGN_REG_CHANGE_MASK) updateRegisters();
#endif

    // Move action flags one bit to the left
    actions = (actions << 1) & AGN_DELAY_MASK;

    // Cancel the event if there is no more work to do
    if (!actions) cancel<AGN_SLOT>();
}

void
Agnus::serviceBPLEvent()
{
    assert(checkTriggeredEvent(BPL_SLOT));

    switch (slot[BPL_SLOT].id) {

        case BPL_H1:
            if (!bplHwStop()) {
                denise->bpldat[PLANE1] = doBitplaneDMA<0>();
                denise->fillShiftRegisters();
            } else {
                INC_DMAPTR(bplpt[PLANE1]);
            }

            if(unlikely(isLastHx(pos.h))) {
                denise->drawHires(16 + denise->scrollHiresOdd);
                addBPLMOD<0>();
            } else {
                denise->drawHires(16);
            }
            break;

        case BPL_L1:
            if (!bplHwStop()) {
                denise->bpldat[PLANE1] = doBitplaneDMA<0>();
                denise->fillShiftRegisters();
            } else {
                INC_DMAPTR(bplpt[PLANE1]);
            }

            if(unlikely(isLastLx(pos.h))) {
                denise->drawLores(16 + denise->scrollLoresOdd);
                addBPLMOD<0>();
            } else {
                denise->drawLores(16);
            }
            break;

        case BPL_H2:
            if (!bplHwStop()) {
                denise->bpldat[PLANE2] = doBitplaneDMA<1>();
            } else {
                INC_DMAPTR(bplpt[PLANE2]);
            }

            if(unlikely(isLastHx(pos.h))) addBPLMOD<1>();
            break;

        case BPL_L2:
            if (!bplHwStop()) {
                denise->bpldat[PLANE2] = doBitplaneDMA<1>();
            } else {
                INC_DMAPTR(bplpt[PLANE2]);
            }

            if(unlikely(isLastLx(pos.h))) addBPLMOD<1>();
            break;

        case BPL_H3:
            if (!bplHwStop()) {
                denise->bpldat[PLANE3] = doBitplaneDMA<2>();
            } else {
                INC_DMAPTR(bplpt[PLANE3]);
            }

            if(unlikely(isLastHx(pos.h))) addBPLMOD<2>();
            break;

        case BPL_L3:
            if (!bplHwStop()) {
                denise->bpldat[PLANE3] = doBitplaneDMA<2>();
            } else {
                INC_DMAPTR(bplpt[PLANE3]);
            }

            if(unlikely(isLastLx(pos.h))) addBPLMOD<2>();
            break;

        case BPL_H4:
            if (!bplHwStop()) {
                denise->bpldat[PLANE4] = doBitplaneDMA<3>();
            } else {
                INC_DMAPTR(bplpt[PLANE4]);
            }

            if(unlikely(isLastHx(pos.h))) addBPLMOD<3>();
            break;

        case BPL_L4:
            if (!bplHwStop()) {
                denise->bpldat[PLANE4] = doBitplaneDMA<3>();
            } else {
                INC_DMAPTR(bplpt[PLANE4]);
            }

            if(unlikely(isLastLx(pos.h))) addBPLMOD<3>();
            break;

        case BPL_L5:
            if (!bplHwStop()) {
                denise->bpldat[PLANE5] = doBitplaneDMA<4>();
            } else {
                INC_DMAPTR(bplpt[PLANE5]);
            }

            if(unlikely(isLastLx(pos.h))) addBPLMOD<4>();
            break;

        case BPL_L6:
            if (!bplHwStop()) {
                denise->bpldat[PLANE6] = doBitplaneDMA<5>();
            } else {
                INC_DMAPTR(bplpt[PLANE6]);
            }

            if(unlikely(isLastLx(pos.h))) addBPLMOD<5>();
            break;

        case BPL_EOL:
            // This is the last event in the current rasterline.
            assert(pos.h == 0xE2);

            // We tell Agnus to call the hsync handler at the beginning of the
            // next cycle and return without scheduling a new BPL event.
            setActionFlag(AGN_HSYNC);
            return;

        default:
            dumpEvents();
            assert(false);
    }

    // Schedule next event
    scheduleNextBplEvent();
}

void
Agnus::serviceDASEvent()
{
    EventID id = slot[DAS_SLOT].id;

    assert(checkTriggeredEvent(DAS_SLOT));
    assert(pos.h == DASEventCycle(id));

    switch (id) {

        case DAS_REFRESH:

            // Block memory refresh DMA cycles
            busOwner[0x01] = BUS_REFRESH;
            busOwner[0x03] = BUS_REFRESH;
            busOwner[0x05] = BUS_REFRESH;
            busOwner[0xE2] = BUS_REFRESH;
            break;

        case DAS_D0:
        case DAS_D1:
        case DAS_D2:

            if (paula->diskController.getFifoBuffering())
                paula->diskController.performDMA();
            else
                paula->diskController.performSimpleDMA();
            break;

        case DAS_A0:
        case DAS_A1:
        case DAS_A2:
        case DAS_A3:
            break;

        case DAS_S0_1:
            executeFirstSpriteCycle<0>();
            break;

        case DAS_S0_2:
            executeSecondSpriteCycle<0>();
            break;

        case DAS_S1_1:
            executeFirstSpriteCycle<1>();
            break;

        case DAS_S1_2:
            executeSecondSpriteCycle<1>();
            break;

        case DAS_S2_1:
            executeFirstSpriteCycle<2>();
            break;

        case DAS_S2_2:
            executeSecondSpriteCycle<2>();
            break;

        case DAS_S3_1:
            executeFirstSpriteCycle<3>();
            break;

        case DAS_S3_2:
            executeSecondSpriteCycle<3>();
            break;

        case DAS_S4_1:
            executeFirstSpriteCycle<4>();
            break;

        case DAS_S4_2:
            executeSecondSpriteCycle<4>();
            break;

        case DAS_S5_1:
            executeFirstSpriteCycle<5>();
            break;

        case DAS_S5_2:
            executeSecondSpriteCycle<5>();
            break;

        case DAS_S6_1:
            executeFirstSpriteCycle<6>();
            break;

        case DAS_S6_2:
            executeSecondSpriteCycle<6>();
            break;

        case DAS_S7_1:
            executeFirstSpriteCycle<7>();
            break;

        case DAS_S7_2:
            executeSecondSpriteCycle<7>();
            break;

        default:
            assert(false);
    }

    // Schedule next event
    int16_t delay = nextDASDelay[id][dmaDAS];
    EventID event = nextDASEvent[id][dmaDAS];
    scheduleRel<DAS_SLOT>(DMA_CYCLES(delay), event);
}

void
Agnus::serviceINSEvent()
{
    assert(checkTriggeredEvent(INS_SLOT));

    switch (slot[INS_SLOT].id) {

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
    rescheduleRel<INS_SLOT>((Cycle)(inspectionInterval * 28000000));
}

void
Agnus::serviceRASEvent()
{
    switch (slot[RAS_SLOT].id) {

        case RAS_HSYNC:
            hsyncHandler();
            break;

        default:
            assert(false);
            break;
    }

    // Reschedule event
    rescheduleRel<RAS_SLOT>(DMA_CYCLES(HPOS_CNT));
}

bool
Agnus::checkScheduledEvent(EventSlot s)
{
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
        case REG_SLOT:
            if (!isRegEvent(id)) {
                _dump();
                panic("Invalid REG event ID.");
                return false;
            }
            break;

        case AGN_SLOT:
            if (!isAgnEvent(id)) {
                _dump();
                panic("Invalid AGN event ID.");
                return false;
            }
            break;

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

        case BPL_SLOT:
            if (!isBplEvent(id)) {
                _dump();
                panic("Invalid BPL event ID.");
                return false;
            }
            break;

        case DAS_SLOT:
            if (!isDasEvent(id)) {
                _dump();
                panic("Invalid DAS event ID.");
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
    switch (s) {

        case AGN_SLOT:
            if (slot[s].id != AGN_ACTIONS) {
                assert(false); return false;
            }
            break;

        default:
            break;
    }

    if (clock < slot[s].triggerCycle) {
        assert(false); return false;
    }
    
    return true;
}
