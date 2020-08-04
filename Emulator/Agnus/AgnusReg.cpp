// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

u16
Agnus::peekDMACONR()
{
    u16 result = dmacon;
    
    assert((result & ((1 << 14) | (1 << 13))) == 0);
    
    if (blitter.isBusy()) result |= (1 << 14);
    if (blitter.isZero()) result |= (1 << 13);
    
    return result;
}

void
Agnus::pokeDMACON(u16 value)
{
    debug(DMA_DEBUG, "pokeDMACON(%X)\n", value);
    
    // Record the change
    // recordRegisterChange(DMA_CYCLES(2), SET_DMACON, value);
    setDMACON(dmacon, value);
}

void
Agnus::setDMACON(u16 oldValue, u16 value)
{
    debug(DMA_DEBUG, "setDMACON(%x, %x)\n", oldValue, value);
    
    // Compute new value
    u16 newValue;
    if (value & 0x8000) {
        newValue = (dmacon | value) & 0x07FF;
    } else {
        newValue = (dmacon & ~value) & 0x07FF;
    }
    
    if (oldValue == newValue) return;
    
    dmacon = newValue;
    
    // Update variable dmaconAtDDFStrt if DDFSTRT has not been reached yet
    if (pos.h + 2 < ddfstrtReached) dmaconAtDDFStrt = newValue;
    
    // Check the lowest 5 bits
    bool oldDMAEN = (oldValue & DMAEN);
    bool oldBPLEN = (oldValue & BPLEN) && oldDMAEN;
    bool oldCOPEN = (oldValue & COPEN) && oldDMAEN;
    bool oldBLTEN = (oldValue & BLTEN) && oldDMAEN;
    bool oldSPREN = (oldValue & SPREN) && oldDMAEN;
    bool oldDSKEN = (oldValue & DSKEN) && oldDMAEN;
    bool oldAUD0EN = (oldValue & AUD0EN) && oldDMAEN;
    bool oldAUD1EN = (oldValue & AUD1EN) && oldDMAEN;
    bool oldAUD2EN = (oldValue & AUD2EN) && oldDMAEN;
    bool oldAUD3EN = (oldValue & AUD3EN) && oldDMAEN;
    
    bool newDMAEN = (newValue & DMAEN);
    bool newBPLEN = (newValue & BPLEN) && newDMAEN;
    bool newCOPEN = (newValue & COPEN) && newDMAEN;
    bool newBLTEN = (newValue & BLTEN) && newDMAEN;
    bool newSPREN = (newValue & SPREN) && newDMAEN;
    bool newDSKEN = (newValue & DSKEN) && newDMAEN;
    bool newAUD0EN = (newValue & AUD0EN) && newDMAEN;
    bool newAUD1EN = (newValue & AUD1EN) && newDMAEN;
    bool newAUD2EN = (newValue & AUD2EN) && newDMAEN;
    bool newAUD3EN = (newValue & AUD3EN) && newDMAEN;
    
    bool toggleBPLEN = oldBPLEN ^ newBPLEN;
    bool toggleCOPEN = oldCOPEN ^ newCOPEN;
    bool toggleBLTEN = oldBLTEN ^ newBLTEN;
    bool toggleSPREN = oldSPREN ^ newSPREN;
    bool toggleDSKEN = oldDSKEN ^ newDSKEN;
    
    // Inform the delegates
    blitter.pokeDMACON(oldValue, newValue);
    
    // Bitplane DMA
    if (toggleBPLEN) {
        
        if (isOCS()) {
            newBPLEN ? enableBplDmaOCS() : disableBplDmaOCS();
        } else {
            newBPLEN ? enableBplDmaECS() : disableBplDmaECS();
        }
        
        hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
    }
    
    // Let Denise know about the change
    denise.pokeDMACON(oldValue, newValue);
    
    // Check DAS DMA (Disk, Audio, Sprites)
    // u16 oldDAS = oldDMAEN ? (oldValue & 0x3F) : 0;
    // u16 newDAS = newDMAEN ? (newValue & 0x3F) : 0;
    // if (oldDAS != newDAS) {
    
    // Disk DMA and sprite DMA
    // We don't need to rebuild the table if audio DMA changes, because
    // audio events are always executed.
    if (toggleDSKEN || toggleSPREN) {
        
        if (toggleSPREN)
            debug(DMA_DEBUG, "Sprite DMA %s\n", newSPREN ? "on" : "off");
        if (toggleDSKEN)
            debug(DMA_DEBUG, "Disk DMA %s\n", newDSKEN ? "on" : "off");
        
        u16 newDAS = newDMAEN ? (newValue & 0x3F) : 0;
        
        // Schedule the DAS DMA table to be rebuild
        hsyncActions |= HSYNC_UPDATE_DAS_TABLE;
        
        // Make the effect visible in the current rasterline as well
        for (int i = pos.h; i < HPOS_CNT; i++) {
            dasEvent[i] = dasDMA[newDAS][i];
        }
        updateDasJumpTable();
        
        // Rectify the currently scheduled DAS event
        scheduleDasEventForCycle(pos.h);
    }
    
    // Copper DMA
    if (toggleCOPEN) {
        debug(DMA_DEBUG, "Copper DMA %s\n", newCOPEN ? "on" : "off");
        if (newCOPEN) copper.activeInThisFrame = true;
    }
    
    // Blitter DMA
    if (toggleBLTEN) {
        debug(DMA_DEBUG, "Blitter DMA %s\n", newBLTEN ? "on" : "off");
    }
    
    // Audio DMA
    if (oldAUD0EN ^ newAUD0EN) {
        newAUD0EN ? audioUnit.channel0.enableDMA() : audioUnit.channel0.disableDMA();
    }
    if (oldAUD1EN ^ newAUD1EN) {
        newAUD1EN ? audioUnit.channel1.enableDMA() : audioUnit.channel1.disableDMA();
    }
    if (oldAUD2EN ^ newAUD2EN) {
        newAUD2EN ? audioUnit.channel2.enableDMA() : audioUnit.channel2.disableDMA();
    }
    if (oldAUD3EN ^ newAUD3EN) {
        newAUD3EN ? audioUnit.channel3.enableDMA() : audioUnit.channel3.disableDMA();
    }
}

u16
Agnus::peekVHPOSR()
{
    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // V7 V6 V5 V4 V3 V2 V1 V0 H8 H7 H6 H5 H4 H3 H2 H1
    
    i16 posh = pos.h + 4;
    i16 posv = pos.v;
    
    // Check if posh has wrapped over (we just added 4)
    if (posh > HPOS_MAX) {
        posh -= HPOS_CNT;
        if (++posv >= frame.numLines()) posv = 0;
    }
    
    // The value of posv only shows up in cycle 2 and later
    if (posh > 1) {
        return HI_LO(posv & 0xFF, posh);
    }
    
    // In cycle 0 and 1, We need to return the old value of posv
    if (posv > 0) {
        return HI_LO((posv - 1) & 0xFF, posh);
    } else {
        return HI_LO(frame.prevLastLine() & 0xFF, posh);
    }
}

void
Agnus::pokeVHPOS(u16 value)
{
    debug(POSREG_DEBUG, "pokeVHPOS(%X)\n", value);
    // Don't know what to do here ...
}

u16
Agnus::peekVPOSR()
{
    u16 id;
    
    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // LF I6 I5 I4 I3 I2 I1 I0 -- -- -- -- -- -- -- V8
    u16 result = (pos.v >> 8) | (frame.isLongFrame() ? 0x8000 : 0);
    assert((result & 0x7FFE) == 0);
    
    // Add identification bits
    switch (config.revision) {
            
        case AGNUS_8367: id = 0x00; break;
        case AGNUS_8372: id = 0x20; break;
        case AGNUS_8375: id = 0x20; break; // TODO: CHECK ON REAL MACHINE
        default: assert(false);
    }
    result |= (id << 8);
    
    debug(POSREG_DEBUG, "peekVPOSR() = %X\n", result);
    return result;
}

void
Agnus::pokeVPOS(u16 value)
{
    debug(XFILES, "XFILES (VPOS): %x (%d,%d)\n", value, pos.v, frame.lof);
    debug(POSREG_DEBUG, "pokeVPOS(%x) (%d,%d)\n", value, pos.v, frame.lof);
    
    // I don't really know what exactly we are supposed to do here.
    // For the time being, I only take care of the LOF bit.
    bool newlof = value & 0x8000;
    if (frame.lof == newlof) return;
    
    // If a long frame gets changed to a short frame, we only proceed if
    // Agnus is not in the last rasterline. Otherwise, we would corrupt the
    // emulators internal state (we would be in a line that is unreachable).
    if (!newlof && inLastRasterline()) return;
    frame.lof = newlof;
    
    // Reschedule a pending VBL_STROBE event with a trigger cycle that is
    // consistent with new LOF bit value.
    if (slot[VBL_SLOT].id == VBL_STROBE0) {
        reschedulePos<VBL_SLOT>(frame.numLines() + vStrobeLine(), 0);
    }
    if (slot[VBL_SLOT].id == VBL_STROBE1) {
        reschedulePos<VBL_SLOT>(frame.numLines() + vStrobeLine(), 1);
    }
}

template <Accessor s> void
Agnus::pokeDIWSTRT(u16 value)
{
    debug(DIW_DEBUG, "pokeDIWSTRT<%s>(%X)\n", AccessorName(s), value);
    recordRegisterChange(DMA_CYCLES(2), SET_DIWSTRT, value);
}

template <Accessor s> void
Agnus::pokeDIWSTOP(u16 value)
{
    debug(DIW_DEBUG, "pokeDIWSTOP<%s>(%X)\n", AccessorName(s), value);
    recordRegisterChange(DMA_CYCLES(2), SET_DIWSTOP, value);
}

void
Agnus::setDIWSTRT(u16 value)
{
    debug(DIW_DEBUG, "setDIWSTRT(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 0, V8 = 0
    
    diwstrt = value;
    
    // Extract the upper left corner of the display window
    i16 newDiwVstrt = HI_BYTE(value);
    i16 newDiwHstrt = LO_BYTE(value);
    
    debug(DIW_DEBUG, "newDiwVstrt = %d newDiwHstrt = %d\n", newDiwVstrt, newDiwHstrt);
    
    // Invalidate the horizontal coordinate if it is out of range
    if (newDiwHstrt < 2) {
        debug(DIW_DEBUG, "newDiwHstrt is too small\n");
        newDiwHstrt = -1;
    }
    
    /* Check if the change already takes effect in the current rasterline.
     *
     *     old: Old trigger coordinate (diwHstrt)
     *     new: New trigger coordinate (newDiwHstrt)
     *     cur: Position of the electron beam (derivable from pos.h)
     *
     * The following cases have to be taken into accout:
     *
     *    1) cur < old < new : Change takes effect in this rasterline.
     *    2) cur < new < old : Change takes effect in this rasterline.
     *    3) new < cur < old : Neither the old nor the new trigger hits.
     *    4) new < old < cur : Already triggered. Nothing to do in this line.
     *    5) old < cur < new : Already triggered. Nothing to do in this line.
     *    6) old < new < cur : Already triggered. Nothing to do in this line.
     */
    
    i16 cur = 2 * pos.h;
    
    // (1) and (2)
    if (cur < diwHstrt && cur < newDiwHstrt) {
        
        debug(DIW_DEBUG, "Updating DIW hflop immediately at %d\n", cur);
        diwHFlopOn = newDiwHstrt;
    }
    
    // (3)
    if (newDiwHstrt < cur && cur < diwHstrt) {
        
        debug(DIW_DEBUG, "DIW hflop not switched on in current line\n");
        diwHFlopOn = -1;
    }
    
    diwVstrt = newDiwVstrt;
    diwHstrt = newDiwHstrt;
    
    /* Update the vertical DIW flipflop
     * This is not 100% accurate. If the vertical DIW flipflop changes in the
     * middle of a rasterline, the effect is immediately visible on a real
     * Amiga. The current emulation code only evaluates the flipflop at the end
     * of the rasterline in the drawing routine of Denise. Hence, the whole
     * line will be blacked out, not just the rest of it.
     */
    if (pos.v == diwVstrt) diwVFlop = true;
    if (pos.v == diwVstop) diwVFlop = false;
}

void
Agnus::setDIWSTOP(u16 value)
{
    debug(DIW_DEBUG, "setDIWSTOP(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 1, V8 = !V7
    
    diwstop = value;
    
    // Extract the lower right corner of the display window
    i16 newDiwVstop = HI_BYTE(value) | ((value & 0x8000) ? 0 : 0x100);
    i16 newDiwHstop = LO_BYTE(value) | 0x100;
    
    debug(DIW_DEBUG, "newDiwVstop = %d newDiwHstop = %d\n", newDiwVstop, newDiwHstop);
    
    // Invalidate the coordinate if it is out of range
    if (newDiwHstop > 0x1C7) {
        debug(DIW_DEBUG, "newDiwHstop is too large\n");
        newDiwHstop = -1;
    }
    
    // Check if the change already takes effect in the current rasterline.
    i16 cur = 2 * pos.h;
    
    // (1) and (2) (see setDIWSTRT)
    if (cur < diwHstop && cur < newDiwHstop) {
        
        debug(DIW_DEBUG, "Updating hFlopOff immediately at %d\n", cur);
        diwHFlopOff = newDiwHstop;
    }
    
    // (3) (see setDIWSTRT)
    if (newDiwHstop < cur && cur < diwHstop) {
        
        debug(DIW_DEBUG, "hFlop not switched off in current line\n");
        diwHFlopOff = -1;
    }
    
    diwVstop = newDiwVstop;
    diwHstop = newDiwHstop;
    
    /* Update the vertical DIW flipflop
     * This is not 100% accurate. See comment in setDIWSTRT().
     */
    if (pos.v == diwVstrt) diwVFlop = true;
    if (pos.v == diwVstop) diwVFlop = false;
}

void
Agnus::pokeDDFSTRT(u16 value)
{
    debug(DDF_DEBUG, "pokeDDFSTRT(%X)\n", value);
    
    //      15 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // OCS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 -- --
    // ECS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 H2 --
    
    value &= ddfMask();
    recordRegisterChange(DMA_CYCLES(2), SET_DDFSTRT, value);
}

void
Agnus::pokeDDFSTOP(u16 value)
{
    debug(DDF_DEBUG, "pokeDDFSTOP(%X)\n", value);
    
    //      15 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // OCS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 -- --
    // ECS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 H2 --
    
    value &= ddfMask();
    recordRegisterChange(DMA_CYCLES(2), SET_DDFSTOP, value);
}

void
Agnus::setDDFSTRT(u16 old, u16 value)
{
    debug(DDF_DEBUG, "setDDFSTRT(%X, %X)\n", old, value);
    
    ddfstrt = value;
    
    // Tell the hsync handler to recompute the DDF window
    hsyncActions |= HSYNC_PREDICT_DDF;
    
    // Take immediate action if we haven't reached the old DDFSTRT cycle yet
    if (pos.h < ddfstrtReached) {
        
        // Check if the new position has already been passed
        if (ddfstrt <= pos.h + 2) {
            
            // DDFSTRT never matches in the current rasterline. Disable DMA
            ddfstrtReached = -1;
            clearBplEvents();
            scheduleNextBplEvent();
            
        } else {
            
            // Update the matching position and recalculate the DMA table
            ddfstrtReached = ddfstrt > HPOS_MAX ? -1 : ddfstrt;
            computeDDFWindow();
            updateBplEvents();
            scheduleNextBplEvent();
        }
    }
}

void
Agnus::setDDFSTOP(u16 old, u16 value)
{
    debug(DDF_DEBUG, "setDDFSTOP(%X, %X)\n", old, value);
    
    ddfstop = value;
    
    // Tell the hsync handler to recompute the DDF window
    hsyncActions |= HSYNC_PREDICT_DDF;
    
    // Take action if we haven't reached the old DDFSTOP cycle yet
    if (pos.h + 2 < ddfstopReached || ddfstopReached == -1) {
        
        // Check if the new position has already been passed
        if (ddfstop <= pos.h + 2) {
            
            // DDFSTOP won't match in the current rasterline
            ddfstopReached = -1;
            
        } else {
            
            // Update the matching position and recalculate the DMA table
            ddfstopReached = (ddfstop > HPOS_MAX) ? -1 : ddfstop;
            if (ddfstrtReached >= 0) {
                computeDDFWindow();
                updateBplEvents();
                scheduleNextBplEvent();
            }
        }
    }
}

void
Agnus::pokeBPLCON0(u16 value)
{
    debug(DMA_DEBUG, "pokeBPLCON0(%X)\n", value);
    
    if (bplcon0 != value) {
        recordRegisterChange(DMA_CYCLES(4), SET_AGNUS_BPLCON0, value);
    }
}

void
Agnus::setBPLCON0(u16 oldValue, u16 newValue)
{
    assert(oldValue != newValue);
    
    debug(DMA_DEBUG, "setBPLCON0(%X,%X)\n", oldValue, newValue);
    
    // Update variable bplcon0AtDDFStrt if DDFSTRT has not been reached yet
    if (pos.h < ddfstrtReached) bplcon0AtDDFStrt = newValue;
    
    // Update the bpl event table in the next rasterline
    hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
    
    // Check if the hires bit or one of the BPU bits have been modified
    if ((oldValue ^ newValue) & 0xF000) {
        
        /*
         debug("oldBplcon0 = %X newBplcon0 = %X\n", oldBplcon0, newBplcon0);
         dumpBplEventTable();
         */
        
        /* TODO:
         * BPLCON0 is usually written in each frame.
         * To speed up, just check the hpos. If it is smaller than the start
         * of the DMA window, a standard update() is enough and the scheduled
         * update in hsyncActions (HSYNC_UPDATE_BPL_TABLE) can be omitted.
         */
        
        // Update the DMA allocation table
        updateBplEvents(dmacon, newValue, pos.h);
        
        // Since the table has changed, we also need to update the event slot
        scheduleBplEventForCycle(pos.h);
    }
    
    bplcon0 = newValue;
}

void
Agnus::pokeBPLCON1(u16 value)
{
    debug(DMA_DEBUG, "pokeBPLCON1(%X)\n", value);
    
    if (bplcon1 != value) {
        recordRegisterChange(DMA_CYCLES(1), SET_AGNUS_BPLCON1, value);
    }
}

void
Agnus::setBPLCON1(u16 oldValue, u16 newValue)
{
    assert(oldValue != newValue);
    debug(DMA_DEBUG, "setBPLCON1(%X,%X)\n", oldValue, newValue);
    
    bplcon1 = newValue & 0xFF;
    
    // Compute comparision values for the hpos counter
    scrollLoresOdd  = (bplcon1 & 0b00001110) >> 1;
    scrollLoresEven = (bplcon1 & 0b11100000) >> 5;
    scrollHiresOdd  = (bplcon1 & 0b00000110) >> 1;
    scrollHiresEven = (bplcon1 & 0b01100000) >> 5;
    
    // Update the bitplane event table starting at the current hpos
    updateBplEvents(pos.h);
    
    // Update the scheduled bitplane event according to the new table
    scheduleBplEventForCycle(pos.h);
    
    // Schedule the bitplane event table to be recomputed
    agnus.hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
    
    // Schedule the DDF window to be recomputed
    hsyncActions |= HSYNC_PREDICT_DDF;
}

template void Agnus::pokeDIWSTRT<CPU_ACCESS>(u16 value);
template void Agnus::pokeDIWSTRT<AGNUS_ACCESS>(u16 value);
template void Agnus::pokeDIWSTOP<CPU_ACCESS>(u16 value);
template void Agnus::pokeDIWSTOP<AGNUS_ACCESS>(u16 value);

