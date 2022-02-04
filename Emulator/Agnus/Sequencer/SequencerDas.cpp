// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Sequencer.h"

EventID Sequencer::dasDMA[64][HPOS_CNT];

void
Sequencer::initDasEventTable()
{
    std::memset(dasDMA, 0, sizeof(dasDMA));

    for (isize enable = 0; enable < 64; enable++) {

        EventID *p = dasDMA[enable];

        p[0x01] = DAS_REFRESH;

        if (enable & DSKEN) {
            
            p[0x07] = DAS_D0;
            p[0x09] = DAS_D1;
            p[0x0B] = DAS_D2;
        }
        
        // Audio DMA is possible even in lines where the DMACON bits are false
        p[0x0D] = DAS_A0;
        p[0x0F] = DAS_A1;
        p[0x11] = DAS_A2;
        p[0x13] = DAS_A3;
        
        if (enable & SPREN) {
            
            p[0x15] = DAS_S0_1;
            p[0x17] = DAS_S0_2;
            p[0x19] = DAS_S1_1;
            p[0x1B] = DAS_S1_2;
            p[0x1D] = DAS_S2_1;
            p[0x1F] = DAS_S2_2;
            p[0x21] = DAS_S3_1;
            p[0x23] = DAS_S3_2;
            p[0x25] = DAS_S4_1;
            p[0x27] = DAS_S4_2;
            p[0x29] = DAS_S5_1;
            p[0x2B] = DAS_S5_2;
            p[0x2D] = DAS_S6_1;
            p[0x2F] = DAS_S6_2;
            p[0x31] = DAS_S7_1;
            p[0x33] = DAS_S7_2;
        }

        p[0xDF] = DAS_SDMA;
        p[0x66] = DAS_TICK;
    }
}

void
Sequencer::initDasEvents()
{
    for (isize i = 0; i < HPOS_CNT; i++) dasEvent[i] = dasDMA[0][i];
    updateDasJumpTable();
}

void
Sequencer::updateDasEvents(u16 dmacon, isize pos)
{
    assert(dmacon < 64);

    // Allocate slots
    for (isize i = pos; i < 0x38; i++) dasEvent[i] = dasDMA[dmacon][i];
    
    // Update the jump table
    updateDasJumpTable(0x38);
}

void
Sequencer::updateDasJumpTable(i16 end)
{
    assert(end <= HPOS_MAX);

    u8 next = nextDasEvent[end];
    
    for (isize i = end; i >= 0; i--) {
        
        nextDasEvent[i] = next;
        if (dasEvent[i]) next = (i8)i;
    }
}
