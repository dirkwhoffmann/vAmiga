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

    debug(2, "peekDMACONR: %X\n", result);
    return result;
}

void
Agnus::pokeDMACON(u16 value)
{
    debug(DMA_DEBUG, "pokeDMACON(%X)\n", value);

    // Record the change
    recordRegisterChange(DMA_CYCLES(2), REG_DMACON, value);
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

    // Inform the delegates
    blitter.pokeDMACON(oldValue, newValue);

    // Bitplane DMA
    if (oldBPLEN ^ newBPLEN) {

        // Update the bpl event table in the next rasterline
        hsyncActions |= HSYNC_UPDATE_BPL_TABLE;

        if (newBPLEN) {

            // Bitplane DMA is switched on

            // Check if the current line is affected by the change
            if (pos.h + 2 < ddfstrtReached || bpldma(dmaconAtDDFStrt)) {

                allocateBplSlots(newValue, bplcon0, pos.h + 2);
                updateBplEvent();
            }

        } else {

            // Bitplane DMA is switched off
            allocateBplSlots(newValue, bplcon0, pos.h + 2);
            updateBplEvent();
        }

        // Let Denise know about the change
        denise.pokeDMACON(oldValue, newValue);
    }

    // Check DAS DMA (Disk, Audio, Sprites)
    u16 oldDAS = oldDMAEN ? (oldValue & 0x3F) : 0;
    u16 newDAS = newDMAEN ? (newValue & 0x3F) : 0;

    if (oldDAS != newDAS) {

        // Schedule the DAS DMA tabel to rebuild
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
    if (oldCOPEN ^ newCOPEN) {
        debug(DMA_DEBUG, "Copper DMA switched %s\n", newCOPEN ? "on" : "off");
    }

    // Blitter DMA
    if (oldBLTEN ^ newBLTEN) {
        debug(DMA_DEBUG, "Blitter DMA switched %s\n", newBLTEN ? "on" : "off");
    }

    // Sprite DMA
    if (oldSPREN ^ newSPREN) {
        debug(DMA_DEBUG, "Sprite DMA switched %s\n", newSPREN ? "on" : "off");
    }

    // Disk DMA
    if (oldDSKEN ^ newDSKEN) {
        debug(DMA_DEBUG, "Disk DMA switched %s\n", newDSKEN ? "on" : "off");
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

void
Agnus::pokeDSKPTH(u16 value)
{
    debug(DSKREG_DEBUG, "pokeDSKPTH(%X)\n", value);
    dskpt = REPLACE_HI_WORD(dskpt, value);
}

void
Agnus::pokeDSKPTL(u16 value)
{
    debug(DSKREG_DEBUG, "pokeDSKPTL(%X)\n", value);
    dskpt = REPLACE_LO_WORD(dskpt, value & 0xFFFE);
}

template <int x> void
Agnus::pokeAUDxLCH(u16 value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dLCH(%X)\n", x, value);

     audlc[x] = REPLACE_HI_WORD(audlc[x], value);
}

template <int x> void
Agnus::pokeAUDxLCL(u16 value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dLCL(%X)\n", x, value);

    audlc[x] = REPLACE_LO_WORD(audlc[x], value & 0xFFFE);
}

bool
Agnus::skipBPLxPT(int x)
{
    /* If a new value is written into BPLxPTL or BPLxPTH, this usually happens
     * as described in the left scenario:
     *
     * 88888888888888889999999999999999      88888888888888889999999999999999
     * 0123456789ABCDEF0123456789ABCDEF      0123456789ABCDEF0123456789ABCDEF
     * .4.2.351.4.2.351.4.2.351.4.2.351      .4.2.351.4.2.351.4.2.351.4.2.351
     *     ^ ^                                     ^ ^
     *     | |                                     | |
     *     | Change takes effect here              | New value is lost
     *     Write to BPLxPT                         Write to BPL1PT
     *
     * The right scenario shows that the new value can get lost under certain
     * circumstances. The following must hold:
     *
     *     (1) There is a Lx or Hx event once cycle after the BPL1PT write.
     *     (2) There is no DMA going on when the write would happen.
     */

    if (isBplxEvent(bplEvent[pos.h + 1], x)) { // (1)

        if (bplEvent[pos.h + 2] == EVENT_NONE) { // (2)

            // debug("skipBPLxPT: Value gets lost\n");
            // dumpBplEventTable();
            return true;
        }
    }

    return false;
}

template <int x> void
Agnus::pokeBPLxPTH(u16 value)
{
    // debug(BPLREG_DEBUG, "pokeBPL%dPTH($%d) (%X)\n", x, value, value);

    // Check if the written value gets lost
    if (skipBPLxPT(x)) {
        // debug("BPLxPTH gets lost\n");
        return;
    }

    // Schedule the register updated
    switch (x) {
        case 1: recordRegisterChange(DMA_CYCLES(2), REG_BPL1PTH, value); break;
        case 2: recordRegisterChange(DMA_CYCLES(2), REG_BPL2PTH, value); break;
        case 3: recordRegisterChange(DMA_CYCLES(2), REG_BPL3PTH, value); break;
        case 4: recordRegisterChange(DMA_CYCLES(2), REG_BPL4PTH, value); break;
        case 5: recordRegisterChange(DMA_CYCLES(2), REG_BPL5PTH, value); break;
        case 6: recordRegisterChange(DMA_CYCLES(2), REG_BPL6PTH, value); break;
    }
}

template <int x> void
Agnus::pokeBPLxPTL(u16 value)
{
    // debug(BPLREG_DEBUG, "pokeBPL%dPTL(%d) ($%X)\n", x, value, value);

    // Check if the written value gets lost
    if (skipBPLxPT(x)) {
        debug(BPLREG_DEBUG, "BPLxPTL gets lost\n");
        return;
    }

    // Schedule the register updated
    switch (x) {
        case 1: recordRegisterChange(DMA_CYCLES(2), REG_BPL1PTL, value); break;
        case 2: recordRegisterChange(DMA_CYCLES(2), REG_BPL2PTL, value); break;
        case 3: recordRegisterChange(DMA_CYCLES(2), REG_BPL3PTL, value); break;
        case 4: recordRegisterChange(DMA_CYCLES(2), REG_BPL4PTL, value); break;
        case 5: recordRegisterChange(DMA_CYCLES(2), REG_BPL5PTL, value); break;
        case 6: recordRegisterChange(DMA_CYCLES(2), REG_BPL6PTL, value); break;
    }
}

template <int x> void
Agnus::setBPLxPTH(u16 value)
{
    debug(BPLREG_DEBUG, "setBPLxPTH(%d, %X)\n", x, value);
    bplpt[x - 1] = REPLACE_HI_WORD(bplpt[x - 1], value);
}

template <int x> void
Agnus::setBPLxPTL(u16 value)
{
    debug(BPLREG_DEBUG, "pokeBPLxPTL(%d, %X)\n", x, value);
    bplpt[x - 1] = REPLACE_LO_WORD(bplpt[x - 1], value & 0xFFFE);
}

void
Agnus::pokeBPL1MOD(u16 value)
{
    debug(BPLREG_DEBUG, "pokeBPL1MOD(%X)\n", value);
    recordRegisterChange(DMA_CYCLES(2), REG_BPL1MOD, value);
}

void
Agnus::setBPL1MOD(u16 value)
{
    debug(BPLREG_DEBUG, "setBPL1MOD(%X)\n", value);
    bpl1mod = (i16)(value & 0xFFFE);
}

void
Agnus::pokeBPL2MOD(u16 value)
{
    debug(BPLREG_DEBUG, "pokeBPL2MOD(%X)\n", value);
    recordRegisterChange(DMA_CYCLES(2), REG_BPL2MOD, value);
}

void
Agnus::setBPL2MOD(u16 value)
{
    debug(BPLREG_DEBUG, "setBPL2MOD(%X)\n", value);
    bpl2mod = (i16)(value & 0xFFFE);
}

template <int x> void
Agnus::pokeSPRxPTH(u16 value)
{
    debug(SPRREG_DEBUG, "pokeSPR%dPTH(%X)\n", x, value);
    sprpt[x] = REPLACE_HI_WORD(sprpt[x], value);
}

template <int x> void
Agnus::pokeSPRxPTL(u16 value)
{
    debug(SPRREG_DEBUG, "pokeSPR%dPTL(%X)\n", x, value);
    sprpt[x] = REPLACE_LO_WORD(sprpt[x], value & 0xFFFE);
}

template <int x> void
Agnus::pokeSPRxPOS(u16 value)
{
    debug(SPRREG_DEBUG, "pokeSPR%dPOS(%X)\n", x, value);

    // Compute the value of the vertical counter that is seen here
    i16 v = (pos.h < 0xDF) ? pos.v : (pos.v + 1);

    // Compute the new vertical start position
    sprVStrt[x] = ((value & 0xFF00) >> 8) | (sprVStrt[x] & 0x0100);

    // Update sprite DMA status
    if (sprVStrt[x] == v) sprDmaState[x] = SPR_DMA_ACTIVE;
    if (sprVStop[x] == v) sprDmaState[x] = SPR_DMA_IDLE;
}

template <int x> void
Agnus::pokeSPRxCTL(u16 value)
{
    debug(SPRREG_DEBUG, "pokeSPR%dCTL(%X)\n", x, value);

    // Compute the value of the vertical counter that is seen here
    i16 v = (pos.h < 0xDF) ? pos.v : (pos.v + 1);

    // Compute the new vertical start and stop position
    sprVStrt[x] = ((value & 0b100) << 6) | (sprVStrt[x] & 0x00FF);
    sprVStop[x] = ((value & 0b010) << 7) | (value >> 8);

    // Update sprite DMA status
    if (sprVStrt[x] == v) sprDmaState[x] = SPR_DMA_ACTIVE;
    if (sprVStop[x] == v) sprDmaState[x] = SPR_DMA_IDLE;
}

template void Agnus::pokeAUDxLCH<0>(u16 value);
template void Agnus::pokeAUDxLCH<1>(u16 value);
template void Agnus::pokeAUDxLCH<2>(u16 value);
template void Agnus::pokeAUDxLCH<3>(u16 value);

template void Agnus::pokeAUDxLCL<0>(u16 value);
template void Agnus::pokeAUDxLCL<1>(u16 value);
template void Agnus::pokeAUDxLCL<2>(u16 value);
template void Agnus::pokeAUDxLCL<3>(u16 value);

template void Agnus::pokeBPLxPTH<1>(u16 value);
template void Agnus::pokeBPLxPTH<2>(u16 value);
template void Agnus::pokeBPLxPTH<3>(u16 value);
template void Agnus::pokeBPLxPTH<4>(u16 value);
template void Agnus::pokeBPLxPTH<5>(u16 value);
template void Agnus::pokeBPLxPTH<6>(u16 value);
template void Agnus::setBPLxPTH<1>(u16 value);
template void Agnus::setBPLxPTH<2>(u16 value);
template void Agnus::setBPLxPTH<3>(u16 value);
template void Agnus::setBPLxPTH<4>(u16 value);
template void Agnus::setBPLxPTH<5>(u16 value);
template void Agnus::setBPLxPTH<6>(u16 value);

template void Agnus::pokeBPLxPTL<1>(u16 value);
template void Agnus::pokeBPLxPTL<2>(u16 value);
template void Agnus::pokeBPLxPTL<3>(u16 value);
template void Agnus::pokeBPLxPTL<4>(u16 value);
template void Agnus::pokeBPLxPTL<5>(u16 value);
template void Agnus::pokeBPLxPTL<6>(u16 value);
template void Agnus::setBPLxPTL<1>(u16 value);
template void Agnus::setBPLxPTL<2>(u16 value);
template void Agnus::setBPLxPTL<3>(u16 value);
template void Agnus::setBPLxPTL<4>(u16 value);
template void Agnus::setBPLxPTL<5>(u16 value);
template void Agnus::setBPLxPTL<6>(u16 value);

template void Agnus::pokeSPRxPTH<0>(u16 value);
template void Agnus::pokeSPRxPTH<1>(u16 value);
template void Agnus::pokeSPRxPTH<2>(u16 value);
template void Agnus::pokeSPRxPTH<3>(u16 value);
template void Agnus::pokeSPRxPTH<4>(u16 value);
template void Agnus::pokeSPRxPTH<5>(u16 value);
template void Agnus::pokeSPRxPTH<6>(u16 value);
template void Agnus::pokeSPRxPTH<7>(u16 value);

template void Agnus::pokeSPRxPTL<0>(u16 value);
template void Agnus::pokeSPRxPTL<1>(u16 value);
template void Agnus::pokeSPRxPTL<2>(u16 value);
template void Agnus::pokeSPRxPTL<3>(u16 value);
template void Agnus::pokeSPRxPTL<4>(u16 value);
template void Agnus::pokeSPRxPTL<5>(u16 value);
template void Agnus::pokeSPRxPTL<6>(u16 value);
template void Agnus::pokeSPRxPTL<7>(u16 value);

template void Agnus::pokeSPRxPOS<0>(u16 value);
template void Agnus::pokeSPRxPOS<1>(u16 value);
template void Agnus::pokeSPRxPOS<2>(u16 value);
template void Agnus::pokeSPRxPOS<3>(u16 value);
template void Agnus::pokeSPRxPOS<4>(u16 value);
template void Agnus::pokeSPRxPOS<5>(u16 value);
template void Agnus::pokeSPRxPOS<6>(u16 value);
template void Agnus::pokeSPRxPOS<7>(u16 value);

template void Agnus::pokeSPRxCTL<0>(u16 value);
template void Agnus::pokeSPRxCTL<1>(u16 value);
template void Agnus::pokeSPRxCTL<2>(u16 value);
template void Agnus::pokeSPRxCTL<3>(u16 value);
template void Agnus::pokeSPRxCTL<4>(u16 value);
template void Agnus::pokeSPRxCTL<5>(u16 value);
template void Agnus::pokeSPRxCTL<6>(u16 value);
template void Agnus::pokeSPRxCTL<7>(u16 value);
