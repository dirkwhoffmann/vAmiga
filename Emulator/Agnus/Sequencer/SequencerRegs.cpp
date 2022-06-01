// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"

template <Accessor s> void
Sequencer::pokeDDFSTRT(u16 value)
{
    trace(DDF_DEBUG, "pokeDDFSTRT(%x)\n", value);
    
    //      15 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // OCS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 -- --
    // ECS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 H2 --
    
    value &= agnus.ddfMask();
    
    // Schedule the write cycle
    agnus.recordRegisterChange(DMA_CYCLES(4), SET_DDFSTRT, value);
}

void
Sequencer::setDDFSTRT(u16 old, u16 value)
{
    trace(DDF_DEBUG | SEQ_DEBUG, "setDDFSTRT(%d, %d)\n", old, value);

    auto posh = agnus.pos.h;
    
    if (old != posh && value != posh) {
        
        if (old == value) {
            trace(SEQ_DEBUG, "setDDFSTRT: Skipping (value does not change)\n");
            return;
        }
    
    } else {
        
        if (posh == old) {
            xfiles("setDDFSTRT: Old value matches trigger position\n");
        }
        if (posh == value) {
            xfiles("setDDFSTRT: New value matches trigger position\n");
        }
    }
    
    ddfstrt = value;
    trace(SEQ_DEBUG, "setDDFSTRT: %04x -> %04x\n", old, value);
    
    // Remove the old start event if it hasn't been reached
    sigRecorder.invalidate(posh, SIG_BPHSTART);
    
    // Add the new start event if it will be reached
    if (ddfstrt > posh) sigRecorder.insert(ddfstrt, SIG_BPHSTART);
    
    // Recompute the event table
    computeBplEventTable(sigRecorder);
}

template <Accessor s> void
Sequencer::pokeDDFSTOP(u16 value)
{
    trace(DDF_DEBUG, "pokeDDFSTOP(%x)\n", value);

    //      15 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // OCS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 -- --
    // ECS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 H2 --
    
    value &= agnus.ddfMask();
    
    // Schedule the write cycle
    agnus.recordRegisterChange(DMA_CYCLES(4), SET_DDFSTOP, value);
}

void
Sequencer::setDDFSTOP(u16 old, u16 value)
{
    trace(DDF_DEBUG | SEQ_DEBUG, "setDDFSTOP(%d, %d)\n", old, value);

    auto posh = agnus.pos.h;
    
    if (old != posh && value != posh) {
        
        if (old == value) {
            trace(SEQ_DEBUG, "setDDFSTOP: Skipping (value does not change)\n");
            return;
        }
    
    } else {
        
        if (posh == old) {
            xfiles("setDDFSTOP: Old value matches trigger position\n");
        }
        if (posh == value) {
            xfiles("setDDFSTOP: New value matches trigger position\n");
        }
    }
    
    ddfstop = value;
    trace(SEQ_DEBUG, "setDDFSTOP: %04x -> %04x\n", old, value);

    // Remove the old stop event if it hasn't been reached
    sigRecorder.invalidate(posh + 1, SIG_BPHSTOP);
    
    // Add the new stop event if it will be reached
    if (ddfstop > posh) sigRecorder.insert(ddfstop, SIG_BPHSTOP);
    
    // Recompute the event table
    computeBplEventTable(sigRecorder);
}

void
Sequencer::setDIWSTRT(u16 value)
{
    trace(DIW_DEBUG | SEQ_DEBUG, "setDIWSTRT(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 -- -- -- -- -- -- -- --  and  V8 = 0
    
    diwstrt = value;
    vstrt = HI_BYTE(value);
    
    if (agnus.pos.v == vstrt && agnus.pos.v != vstop) {

        sigRecorder.insert(agnus.pos.h + 2, SIG_VFLOP_SET);
        computeBplEventTable(sigRecorder);
    }
}

void
Sequencer::setDIWSTOP(u16 value)
{
    trace(DIW_DEBUG | SEQ_DEBUG, "setDIWSTOP(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 -- -- -- -- -- -- -- --  and  V8 = !V7
    
    diwstop = value;
    vstop = HI_BYTE(value) | ((value & 0x8000) ? 0 : 0x100);
    
    if (agnus.pos.v == vstop) {

        sigRecorder.insert(agnus.pos.h + 2, SIG_VFLOP_CLR);
        computeBplEventTable(sigRecorder);
    }

    if (agnus.pos.v != vstop && agnus.pos.v == vstrt) {

        sigRecorder.insert(agnus.pos.h + 2, SIG_VFLOP_SET);
        computeBplEventTable(sigRecorder);
    }
}

template void Sequencer::pokeDDFSTRT<ACCESSOR_CPU>(u16 value);
template void Sequencer::pokeDDFSTRT<ACCESSOR_AGNUS>(u16 value);
template void Sequencer::pokeDDFSTOP<ACCESSOR_CPU>(u16 value);
template void Sequencer::pokeDDFSTOP<ACCESSOR_AGNUS>(u16 value);
